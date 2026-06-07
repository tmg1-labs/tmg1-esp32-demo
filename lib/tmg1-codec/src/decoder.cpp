#include "tmg1/decoder.h"
#include "tmg1/range_decoder.h"
#include "tmg1/rice_reader.h"
#include "tmg1/prediction.h"
#include <new>
#include <string.h>

namespace tmg1 {

Decoder::Decoder() : _rangeModel(2, 2) {}

Decoder::~Decoder() {
    delete[] _previousFrame;
    delete[] _payloadBuffer;
    delete[] _tempFrame;
}

Error Decoder::begin(Tmg1Stream& stream) {
    _stream = &stream;

    Error err = readBytes(&_header, sizeof(_header));
    if (err != Error::None) return Error::ReadError;

    if (_header.signature != TMG1_SIGNATURE) return Error::InvalidSignature;
    if (_header.version   != TMG1_VERSION)   return Error::InvalidVersion;

    _frameBufferSize = ((size_t)_header.width * _header.height + 7) / 8;

    if (!_previousFrame) {
        _previousFrame = new (std::nothrow) uint8_t[_frameBufferSize];
        if (!_previousFrame) return Error::InvalidData;
    }
    if (!_tempFrame) {
        _tempFrame     = new (std::nothrow) uint8_t[_frameBufferSize];
        _tempFrameSize = _frameBufferSize;
        if (!_tempFrame) return Error::InvalidData;
    }

    _rangeModel.reset();
    return Error::None;
}

Error Decoder::decodeFrame(uint8_t* buffer, size_t bufferSize) {
    if (bufferSize < _frameBufferSize) return Error::BufferTooSmall;

    FrameHeader fh = {};
    Error err = readFrameHeader(fh);
    if (err != Error::None) return err;
    _lastPtsDelta = fh.ptsDelta;

    // ペイロードが空 (変化なしフレーム)
    if (fh.payloadSize == 0) {
        if (fh.frameType == FRAME_TYPE_P) {
            memcpy(buffer, _previousFrame, _frameBufferSize);
        } else {
            memset(buffer, 0, _frameBufferSize);
            memcpy(_previousFrame, buffer, _frameBufferSize);
        }
        return Error::None;
    }

    // ペイロードをバッファへ読み込む
    if (_payloadBufSize < fh.payloadSize) {
        delete[] _payloadBuffer;
        _payloadBuffer  = new (std::nothrow) uint8_t[fh.payloadSize];
        _payloadBufSize = fh.payloadSize;
        if (!_payloadBuffer) return Error::InvalidData;
    }
    err = readBytes(_payloadBuffer, fh.payloadSize);
    if (err != Error::None) return err;

    bool ok = false;
    if (fh.frameType == FRAME_TYPE_P) {
        // P-Frame: デルタをtempFrameへ展開し、previousFrame とXOR
        ok = decompressPayload(_payloadBuffer, fh.payloadSize, _tempFrame, _frameBufferSize, fh);
        if (ok) {
            applyInversePrediction(_tempFrame, _frameBufferSize,
                                   _header.width, _header.height,
                                   fh.predictionMethod);
            for (size_t i = 0; i < _frameBufferSize; ++i) {
                buffer[i] = _previousFrame[i] ^ _tempFrame[i];
            }
        }
    } else {
        // I-Frame: 直接バッファへ展開
        ok = decompressPayload(_payloadBuffer, fh.payloadSize, buffer, bufferSize, fh);
        if (ok) {
            applyInversePrediction(buffer, bufferSize,
                                   _header.width, _header.height,
                                   fh.predictionMethod);
        }
    }

    if (!ok) return Error::InvalidData;

    memcpy(_previousFrame, buffer, _frameBufferSize);
    return Error::None;
}

Error Decoder::readFrameHeader(FrameHeader& fh) {
    Error err = readBytes(&fh.frameType, 1);
    if (err != Error::None) return err;

    err = readUleb128(fh.ptsDelta);
    if (err != Error::None) return err;

    err = readUleb128(fh.payloadSize);
    if (err != Error::None) return err;

    err = readBytes(&fh.frameFlags, 1);
    if (err != Error::None) return err;

    err = readBytes(&fh.predictionMethod, 1);
    if (err != Error::None) return err;

    return Error::None;
}

Error Decoder::readUleb128(uint32_t& value) {
    value = 0;
    uint8_t shift = 0;
    uint8_t byte;
    while (shift < 35) { // uint32 に収まる上限
        Error err = readBytes(&byte, 1);
        if (err != Error::None) return err;
        value |= (uint32_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) return Error::None;
        shift += 7;
    }
    return Error::ReadError; // ULEB128が5バイトを超えた (不正データ)
}

bool Decoder::decompressPayload(const uint8_t* src, size_t srcSize,
                                uint8_t* dest, size_t destSize,
                                const FrameHeader& fh)
{
    bool useRange = (_header.flags & FLAG_RANGE_CODER) != 0;
    if (useRange) {
        return decompressPayloadRange(src, srcSize, dest, destSize, fh);
    } else {
        return decompressPayloadRice(src, srcSize, dest, destSize, fh);
    }
}

bool Decoder::decompressPayloadRange(const uint8_t* src, size_t srcSize,
                                     uint8_t* dest, size_t destSize,
                                     const FrameHeader& fh)
{
    _rangeModel.reset();
    RangeDecoder reader(src, srcSize, _rangeModel);

    uint16_t width  = _header.width;
    uint16_t height = _header.height;
    size_t bytesPerLine = (width + 7) / 8;
    bool msbFirst = (_header.flags & FLAG_MSB_FIRST) != 0;

    memset(dest, 0, destSize);

    for (uint16_t y = 0; y < height; ++y) {
        if (reader.isEndOfStream()) break;

        // 行タイプビット: 0=空行, 1=データあり
        if (reader.readBit() == 0) continue;

        for (uint16_t x = 0; x < width; ++x) {
            if (reader.isEndOfStream()) break;
            if (reader.readBit() == 0) continue;

            size_t byteIdx = y * bytesPerLine + x / 8;
            uint8_t bitPos = msbFirst ? (7 - x % 8) : (x % 8);
            dest[byteIdx] |= (uint8_t)(1 << bitPos);
        }
    }
    return true;
}

bool Decoder::decompressPayloadRice(const uint8_t* src, size_t srcSize,
                                    uint8_t* dest, size_t destSize,
                                    const FrameHeader& fh)
{
    bool msbFirst    = (_header.flags & FLAG_MSB_FIRST) != 0;
    bool hasStartBit = (fh.frameFlags & FRAME_FLAG_START_BIT) != 0;
    bool perLineK    = (fh.frameFlags & FRAME_FLAG_PER_LINE_K) != 0;
    bool perFrameK   = (fh.frameFlags & FRAME_FLAG_PER_FRAME_K) != 0;

    RiceBitReader reader(src, srcSize, msbFirst);

    int frameK = 1;
    if (perFrameK) {
        uint32_t k = reader.readBits(3);
        if (k == 0xFFFFFFFFu) return false;
        frameK = (int)k;
    }

    uint16_t width  = _header.width;
    uint16_t height = _header.height;
    size_t bytesPerLine = (width + 7) / 8;

    memset(dest, 0, destSize);

    for (uint16_t y = 0; y < height; ++y) {
        int lineType = reader.readBit();
        if (lineType == -1) return false;
        if (lineType == 0) continue;

        int currentBit = 0;
        if (hasStartBit) {
            int b = reader.readBit();
            if (b == -1) return false;
            currentBit = b;
        }

        int lineK = frameK;
        if (perLineK) {
            uint32_t k = reader.readBits(3);
            if (k == 0xFFFFFFFFu) return false;
            lineK = (int)k;
        }

        uint16_t written = 0;
        while (written < width) {
            if (reader.isEndOfStream()) {
                if (currentBit == 1) return false; // 白ランが途中で終わった
                break;
            }

            uint32_t runLen = reader.readSymbol(lineK);
            if (runLen == 0xFFFFFFFFu) return false;

            if (runLen == 0) {
                currentBit = 1 - currentBit;
                continue;
            }

            if (currentBit == 1) {
                for (uint32_t i = 0; i < runLen && written < width; ++i) {
                    size_t byteIdx = y * bytesPerLine + written / 8;
                    uint8_t bitPos = msbFirst ? (7 - written % 8) : (written % 8);
                    dest[byteIdx] |= (uint8_t)(1 << bitPos);
                    written++;
                }
            } else {
                uint32_t skip = (runLen < (uint32_t)(width - written))
                              ? runLen : (uint32_t)(width - written);
                written += (uint16_t)skip;
            }

            if (written > width) written = width;
            currentBit = 1 - currentBit;
        }
        if (!hasStartBit) currentBit = 0;
    }
    return true;
}

Error Decoder::readBytes(void* dst, size_t len) {
    int n = _stream->read(_stream->ctx, static_cast<uint8_t*>(dst), len);
    if (n < 0 || (size_t)n != len) return Error::ReadError;
    return Error::None;
}

} // namespace tmg1
