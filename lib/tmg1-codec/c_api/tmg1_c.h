#pragma once
#include <stddef.h>
#include <stdint.h>
#include "tmg1/io.h"

// Rust FFI 向け extern "C" API
// bindgen でバインディングを自動生成できる形式

#ifdef __cplusplus
extern "C" {
#endif

// エンコード設定 (C構造体)
typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t timebaseNum;
    uint16_t timebaseDen;
    uint16_t keyInterval;
    uint8_t  msbFirst;      // 0 or 1
    uint8_t  useRangeCoder; // 0 or 1
    uint8_t  deltaEnabled;  // 0 or 1
    uint8_t  _pad;          // アライメント用
} Tmg1EncodeConfig;

// --- デコーダ ---

typedef struct Tmg1DecoderOpaque Tmg1Decoder;

// デコーダを生成してファイルヘッダを読む。失敗時は NULL。
Tmg1Decoder* tmg1_decoder_create(Tmg1Stream* stream);

// デコーダを破棄する。
void tmg1_decoder_destroy(Tmg1Decoder* dec);

// 次フレームをバッファへデコードする。
// 戻り値: 0=成功, 負値=エラーコード
int tmg1_decoder_decode_frame(Tmg1Decoder* dec, uint8_t* out, size_t out_size);

uint16_t tmg1_decoder_width          (const Tmg1Decoder* dec);
uint16_t tmg1_decoder_height         (const Tmg1Decoder* dec);
uint16_t tmg1_decoder_timebase_num   (const Tmg1Decoder* dec);
uint16_t tmg1_decoder_timebase_den   (const Tmg1Decoder* dec);
uint32_t tmg1_decoder_last_pts_delta (const Tmg1Decoder* dec);

// --- エンコーダ ---

typedef struct Tmg1EncoderOpaque Tmg1Encoder;

// エンコーダを生成してファイルヘッダを書く。失敗時は NULL。
Tmg1Encoder* tmg1_encoder_create(Tmg1Stream* stream, const Tmg1EncodeConfig* config);

// エンコーダを破棄する。
void tmg1_encoder_destroy(Tmg1Encoder* enc);

// 1フレームをエンコードして書き込む。
// 戻り値: 0=成功, 負値=エラーコード
int tmg1_encoder_encode_frame(Tmg1Encoder* enc, const uint8_t* frame, size_t frame_size);

// ストリームを終了する。
int tmg1_encoder_finish(Tmg1Encoder* enc);

#ifdef __cplusplus
}
#endif
