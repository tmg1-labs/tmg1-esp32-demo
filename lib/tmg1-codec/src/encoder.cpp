#include "tmg1/encoder.h"
#include "tmg1/range_encoder.h"
#include "tmg1/rice_writer.h"
#include "tmg1/prediction.h"
#include <new>
#include <string.h>
#include <vector>

namespace tmg1 {

Encoder::Encoder(const EncodeConfig& config)
    : _config(config), _rangeModel(2, 2)
{
    _frameBufferSize = ((size_t)config.width * config.height + 7) / 8;
}

Encoder::~Encoder() {
    delete[] _previousFrame;
}

Error Encoder::begin(Tmg1Stream& stream) {
    _stream = &stream;

    if (!_previousFrame) {
        _previousFrame = new (std::nothrow) uint8_t[_frameBufferSize];
        if (!_previousFrame) return Error::InvalidData;
        memset(_previousFrame, 0, _frameBufferSize);
    }

    _hasPrevious = false;
    _frameCount  = 0;
    _rangeModel.reset();

    return writeFileHeader();
}

Error Encoder::encodeFrame(const uint8_t* frame, size_t frameSize) {
    if (frameSize < _frameBufferSize) return Error::BufferTooSmall;

    _frameCount++;
    bool isKeyFrame = !_hasPrevious
        || (_config.keyInterval > 0 && _frameCount % _config.keyInterval == 0)
        || !_config.deltaEnabled;

    // P-Frame の場合はデルタを計算
    std::vector<uint8_t> payload(_frameBufferSize);
    if (isKeyFrame) {
        memcpy(payload.data(), frame, _frameBufferSize);
    } else {
        for (size_t i = 0; i < _frameBufferSize; ++i) {
            payload[i] = frame[i] ^ _previousFrame[i];
        }
    }

    // 予測フィルタは None のみサポート (シンプル化)
    // 今後: 最良の予測方式を選択する機能を追加予定
    uint8_t predMethod = PREDICTION_NONE;

    // ペイロードを圧縮
    std::vector<uint8_t> compressed(_frameBufferSize * 2 + 64);
    size_t compressedSize = compressed.size();
    bool ok;
    FrameHeader fh = {};
    fh.frameType        = isKeyFrame ? FRAME_TYPE_I : FRAME_TYPE_P;
    fh.ptsDelta         = 1;
    fh.predictionMethod = predMethod;
    fh.frameFlags       = 0;

    if (_config.useRangeCoder) {
        ok = compressPayloadRange(payload.data(), _frameBufferSize,
                                  compressed.data(), compressedSize, fh);
    } else {
        ok = compressPayloadRice(payload.data(), _frameBufferSize,
                                 compressed.data(), compressedSize, fh);
    }
    if (!ok) return Error::WriteError;

    fh.payloadSize = (uint32_t)compressedSize;

    Error err = writeFrameHeader(fh);
    if (err != Error::None) return err;

    err = writeBytes(compressed.data(), compressedSize);
    if (err != Error::None) return err;

    memcpy(_previousFrame, frame, _frameBufferSize);
    _hasPrevious = true;
    return Error::None;
}

Error Encoder::finish() {
    return Error::None; // 将来: 末尾マーカー書き込みなど
}

Error Encoder::writeFileHeader() {
    FileHeader hdr = {};
    hdr.signature   = TMG1_SIGNATURE;
    hdr.version     = TMG1_VERSION;
    hdr.flags       = 0;
    if (_config.msbFirst)      hdr.flags |= FLAG_MSB_FIRST;
    if (_config.useRangeCoder) hdr.flags |= FLAG_RANGE_CODER;
    hdr.width       = _config.width;
    hdr.height      = _config.height;
    hdr.timebaseNum = _config.timebaseNum;
    hdr.timebaseDen = _config.timebaseDen;
    hdr.keyInterval = _config.keyInterval;
    return writeBytes(&hdr, sizeof(hdr));
}

Error Encoder::writeFrameHeader(const FrameHeader& fh) {
    Error err = writeBytes(&fh.frameType, 1);
    if (err != Error::None) return err;

    err = writeUleb128(fh.ptsDelta);
    if (err != Error::None) return err;

    err = writeUleb128(fh.payloadSize);
    if (err != Error::None) return err;

    err = writeBytes(&fh.frameFlags, 1);
    if (err != Error::None) return err;

    err = writeBytes(&fh.predictionMethod, 1);
    if (err != Error::None) return err;

    return Error::None;
}

Error Encoder::writeUleb128(uint32_t value) {
    do {
        uint8_t byte = (uint8_t)(value & 0x7F);
        value >>= 7;
        if (value != 0) byte |= 0x80;
        Error err = writeBytes(&byte, 1);
        if (err != Error::None) return err;
    } while (value != 0);
    return Error::None;
}

Error Encoder::writeBytes(const void* src, size_t len) {
    int n = _stream->write(_stream->ctx, static_cast<const uint8_t*>(src), len);
    if (n < 0 || (size_t)n != len) return Error::WriteError;
    return Error::None;
}

bool Encoder::compressPayloadRange(const uint8_t* src, size_t srcSize,
                                   uint8_t* dest, size_t& destSize,
                                   const FrameHeader& fh)
{
    Tmg1MemWriteCtx ctx = { dest, destSize, 0 };
    Tmg1Stream out = { &ctx, nullptr, tmg1_mem_write, nullptr, nullptr };

    _rangeModel.reset();
    RangeEncoder writer(_rangeModel, out);

    uint16_t width  = _config.width;
    uint16_t height = _config.height;
    size_t bytesPerLine = (width + 7) / 8;

    for (uint16_t y = 0; y < height; ++y) {
        // 行が全ゼロかチェック
        bool anyBit = false;
        for (size_t i = 0; i < bytesPerLine && !anyBit; ++i) {
            anyBit = (src[y * bytesPerLine + i] != 0);
        }

        writer.writeBit(anyBit ? 1 : 0);
        if (!anyBit) continue;

        for (uint16_t x = 0; x < width; ++x) {
            size_t byteIdx = y * bytesPerLine + x / 8;
            uint8_t bitPos = _config.msbFirst ? (7 - x % 8) : (x % 8);
            int bit = (src[byteIdx] >> bitPos) & 1;
            writer.writeBit(bit);
        }
    }

    if (writer.flush() < 0) return false;
    destSize = ctx.pos;
    return true;
}

bool Encoder::compressPayloadRice(const uint8_t* src, size_t srcSize,
                                  uint8_t* dest, size_t& destSize,
                                  const FrameHeader& fh)
{
    Tmg1MemWriteCtx ctx = { dest, destSize, 0 };
    Tmg1Stream out = { &ctx, nullptr, tmg1_mem_write, nullptr, nullptr };

    RiceBitWriter writer(out, _config.msbFirst);

    uint16_t width  = _config.width;
    uint16_t height = _config.height;
    size_t bytesPerLine = (width + 7) / 8;

    for (uint16_t y = 0; y < height; ++y) {
        bool anyBit = false;
        for (size_t i = 0; i < bytesPerLine && !anyBit; ++i) {
            anyBit = (src[y * bytesPerLine + i] != 0);
        }

        writer.writeBit(anyBit ? 1 : 0);
        if (!anyBit) continue;

        // シンプルなランレングス符号化 (k=1 固定)
        int currentBit = 0;
        uint32_t runLen = 0;

        for (uint16_t x = 0; x <= width; ++x) {
            int bit = 0;
            if (x < width) {
                size_t byteIdx = y * bytesPerLine + x / 8;
                uint8_t bitPos = _config.msbFirst ? (7 - x % 8) : (x % 8);
                bit = (src[byteIdx] >> bitPos) & 1;
            }

            if (x == width || bit != currentBit) {
                writer.writeSymbol(runLen, 1);
                runLen = 0;
                currentBit = 1 - currentBit;
            } else {
                runLen++;
            }
        }
    }

    if (writer.flush() < 0) return false;
    destSize = ctx.pos;
    return true;
}

} // namespace tmg1
