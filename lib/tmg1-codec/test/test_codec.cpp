#include <unity.h>
#include <stdint.h>
#include <string.h>
#include "tmg1/encoder.h"
#include "tmg1/decoder.h"
#include "tmg1/io.h"
#include "tmg1/types.h"

// エンコードバッファ (十分なサイズ)
static uint8_t gEncBuf[4096];
static Tmg1MemWriteCtx gEncCtx;
static Tmg1Stream      gEncStream;

static void setupEnc() {
    memset(gEncBuf, 0, sizeof(gEncBuf));
    gEncCtx    = { gEncBuf, sizeof(gEncBuf), 0 };
    gEncStream = { &gEncCtx, nullptr, tmg1_mem_write, nullptr, nullptr };
}

// 単純なラウンドトリップテスト (Rangeコーダ, 8x8)
void test_codec_range_iframes_roundtrip() {
    const uint16_t W = 8, H = 8;
    const size_t frameSize = (W * H + 7) / 8; // 8バイト

    // 2フレームのテストデータ
    uint8_t frame0[8] = { 0xAA, 0x55, 0xFF, 0x00, 0x0F, 0xF0, 0xCC, 0x33 };
    uint8_t frame1[8] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };

    setupEnc();

    // エンコード
    {
        tmg1::EncodeConfig cfg = {};
        cfg.width         = W;
        cfg.height        = H;
        cfg.timebaseNum   = 1;
        cfg.timebaseDen   = 10;
        cfg.keyInterval   = 0; // すべてI-Frame
        cfg.msbFirst      = true;
        cfg.useRangeCoder = true;
        cfg.deltaEnabled  = false;

        tmg1::Encoder enc(cfg);
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.begin(gEncStream));
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.encodeFrame(frame0, frameSize));
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.encodeFrame(frame1, frameSize));
        enc.finish();
    }

    // デコード
    {
        Tmg1MemReadCtx decCtx = { gEncBuf, gEncCtx.pos, 0 };
        Tmg1Stream decStream  = { &decCtx, tmg1_mem_read, nullptr, nullptr, nullptr };

        tmg1::Decoder dec;
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.begin(decStream));
        TEST_ASSERT_EQUAL_UINT16(W, dec.getWidth());
        TEST_ASSERT_EQUAL_UINT16(H, dec.getHeight());

        uint8_t out[8];
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.decodeFrame(out, sizeof(out)));
        TEST_ASSERT_EQUAL_MEMORY(frame0, out, frameSize);

        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.decodeFrame(out, sizeof(out)));
        TEST_ASSERT_EQUAL_MEMORY(frame1, out, frameSize);
    }
}

// Riceコーダでのラウンドトリップ
void test_codec_rice_iframes_roundtrip() {
    const uint16_t W = 8, H = 8;
    const size_t frameSize = (W * H + 7) / 8;

    uint8_t frame0[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    setupEnc();

    {
        tmg1::EncodeConfig cfg = {};
        cfg.width         = W;
        cfg.height        = H;
        cfg.timebaseNum   = 1;
        cfg.timebaseDen   = 10;
        cfg.keyInterval   = 0;
        cfg.msbFirst      = true;
        cfg.useRangeCoder = false; // Riceコーダ
        cfg.deltaEnabled  = false;

        tmg1::Encoder enc(cfg);
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.begin(gEncStream));
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.encodeFrame(frame0, frameSize));
        enc.finish();
    }

    {
        Tmg1MemReadCtx decCtx = { gEncBuf, gEncCtx.pos, 0 };
        Tmg1Stream decStream  = { &decCtx, tmg1_mem_read, nullptr, nullptr, nullptr };

        tmg1::Decoder dec;
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.begin(decStream));

        uint8_t out[8];
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.decodeFrame(out, sizeof(out)));
        TEST_ASSERT_EQUAL_MEMORY(frame0, out, frameSize);
    }
}

// 全黒フレーム (エッジケース)
void test_codec_all_black_frame() {
    const uint16_t W = 16, H = 8;
    const size_t frameSize = (W * H + 7) / 8; // 16バイト

    uint8_t frame[16] = {};

    setupEnc();

    {
        tmg1::EncodeConfig cfg = {};
        cfg.width         = W;
        cfg.height        = H;
        cfg.timebaseNum   = 1;
        cfg.timebaseDen   = 10;
        cfg.msbFirst      = true;
        cfg.useRangeCoder = true;
        cfg.deltaEnabled  = false;

        tmg1::Encoder enc(cfg);
        enc.begin(gEncStream);
        enc.encodeFrame(frame, frameSize);
        enc.finish();
    }

    {
        Tmg1MemReadCtx decCtx = { gEncBuf, gEncCtx.pos, 0 };
        Tmg1Stream decStream  = { &decCtx, tmg1_mem_read, nullptr, nullptr, nullptr };

        tmg1::Decoder dec;
        dec.begin(decStream);

        uint8_t out[16];
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.decodeFrame(out, sizeof(out)));
        TEST_ASSERT_EQUAL_MEMORY(frame, out, frameSize);
    }
}

// ファイルヘッダ検証: 不正シグネチャ
void test_codec_invalid_signature() {
    uint8_t bad[] = { 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x00, 0x08, 0x00, 1, 10, 0, 0 };
    Tmg1MemReadCtx ctx = { bad, sizeof(bad), 0 };
    Tmg1Stream s = { &ctx, tmg1_mem_read, nullptr, nullptr, nullptr };
    tmg1::Decoder dec;
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::InvalidSignature, (int)dec.begin(s));
}

// ファイルヘッダ検証: 不正バージョン
void test_codec_invalid_version() {
    // 'TMG1' シグネチャ + version=1 (v2でないのでエラー)
    uint8_t bad[] = { 0x54, 0x4D, 0x47, 0x31, 0x01, 0x00, 0x08, 0x00, 0x08, 0x00, 1, 10, 0, 0 };
    Tmg1MemReadCtx ctx = { bad, sizeof(bad), 0 };
    Tmg1Stream s = { &ctx, tmg1_mem_read, nullptr, nullptr, nullptr };
    tmg1::Decoder dec;
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::InvalidVersion, (int)dec.begin(s));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_codec_range_iframes_roundtrip);
    RUN_TEST(test_codec_rice_iframes_roundtrip);
    RUN_TEST(test_codec_all_black_frame);
    RUN_TEST(test_codec_invalid_signature);
    RUN_TEST(test_codec_invalid_version);
    return UNITY_END();
}
