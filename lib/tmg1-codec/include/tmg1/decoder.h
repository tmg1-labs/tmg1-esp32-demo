#pragma once
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "types.h"
#include "freq_model.h"

namespace tmg1 {

// TMG1 v2 デコーダ
// Tmg1Stream 経由で入力を読み、フレームバッファへデコードする
class Decoder {
public:
    Decoder();
    ~Decoder();

    // ストリームを開いてファイルヘッダを読む。失敗時は Error::* を返す。
    Error begin(Tmg1Stream& stream);

    // 次のフレームをバッファへデコードする。bufferSize >= width*height/8 が必要。
    Error decodeFrame(uint8_t* buffer, size_t bufferSize);

    uint16_t getWidth()       const { return _header.width; }
    uint16_t getHeight()      const { return _header.height; }
    uint16_t getTimebaseNum() const { return _header.timebaseNum; }
    uint16_t getTimebaseDen() const { return _header.timebaseDen; }
    uint32_t getLastPtsDelta() const { return _lastPtsDelta; }

private:
    Error readFrameHeader(FrameHeader& fh);
    Error readUleb128(uint32_t& value);

    bool decompressPayload(const uint8_t* src, size_t srcSize,
                           uint8_t* dest, size_t destSize,
                           const FrameHeader& fh);
    bool decompressPayloadRice (const uint8_t* src, size_t srcSize,
                                uint8_t* dest, size_t destSize,
                                const FrameHeader& fh);
    bool decompressPayloadRange(const uint8_t* src, size_t srcSize,
                                uint8_t* dest, size_t destSize,
                                const FrameHeader& fh);

    // ストリームから len バイト読む。不足時は Error::ReadError。
    Error readBytes(void* dst, size_t len);

    Tmg1Stream* _stream          = nullptr;
    FileHeader  _header          = {};
    uint8_t*    _previousFrame   = nullptr;
    size_t      _frameBufferSize = 0;
    uint8_t*    _payloadBuffer   = nullptr;
    size_t      _payloadBufSize  = 0;
    uint8_t*    _tempFrame       = nullptr;
    size_t      _tempFrameSize   = 0;
    uint32_t    _lastPtsDelta    = 0;
    FrequencyModel _rangeModel;
};

} // namespace tmg1
