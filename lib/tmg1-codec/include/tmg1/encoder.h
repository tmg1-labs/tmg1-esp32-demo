#pragma once
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "types.h"
#include "freq_model.h"

namespace tmg1 {

// TMG1 v2 エンコーダ
// デスクトップ / Rust CLI 向け。Tmg1Stream 経由で出力する。
class Encoder {
public:
    explicit Encoder(const EncodeConfig& config);
    ~Encoder();

    // ストリームを開いてファイルヘッダを書く
    Error begin(Tmg1Stream& stream);

    // 1フレーム分のビットパックデータ (width*height/8バイト) をエンコードして書き込む
    Error encodeFrame(const uint8_t* frame, size_t frameSize);

    // ストリームを終了する (現時点では何もしない。将来のフラッシュ用)
    Error finish();

private:
    Error writeFileHeader();
    Error writeFrameHeader(const FrameHeader& fh);
    Error writeUleb128(uint32_t value);
    Error writeBytes(const void* src, size_t len);

    // ペイロードを Range/Rice で圧縮する。dest は呼び出し側が確保 (十分大きいサイズ)。
    // 実際の書き込みサイズを destSize に返す。
    bool compressPayloadRange(const uint8_t* src, size_t srcSize,
                              uint8_t* dest, size_t& destSize,
                              const FrameHeader& fh);
    bool compressPayloadRice (const uint8_t* src, size_t srcSize,
                              uint8_t* dest, size_t& destSize,
                              const FrameHeader& fh);

    EncodeConfig _config;
    Tmg1Stream*  _stream          = nullptr;
    uint8_t*     _previousFrame   = nullptr;
    size_t       _frameBufferSize = 0;
    bool         _hasPrevious     = false;
    uint32_t     _frameCount      = 0;
    FrequencyModel _rangeModel;
};

} // namespace tmg1
