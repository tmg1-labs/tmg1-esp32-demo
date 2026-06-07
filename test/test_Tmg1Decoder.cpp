#include <string.h>
#include <unity.h>
#include "tmg1/decoder.h"
#include "tmg1/encoder.h"
#include "tmg1/io.h"
#include "tmg1/types.h"

void setUp(void)  {}
void tearDown(void) {}

// --- ヘルパー ---

static uint8_t gEncBuf[4096];
static uint8_t gDecBuf[4096];

static Tmg1Stream makeReadStream(const uint8_t* data, size_t size) {
    static Tmg1MemReadCtx ctx;
    ctx = { data, size, 0 };
    Tmg1Stream s = {};
    s.ctx  = &ctx;
    s.read = tmg1_mem_read;
    return s;
}

// --- デコーダ作成テスト ---

void test_Tmg1Decoder_instance_creation() {
    tmg1::Decoder dec;
    TEST_ASSERT_EQUAL_UINT16(0, dec.getWidth());
    TEST_ASSERT_EQUAL_UINT16(0, dec.getHeight());
}

// --- ファイルヘッダ検証 ---

// v2 有効ヘッダ: TMG1 + version=2 + 128x64 + timebase 1/30 + keyInterval=60
static const uint8_t validHeader[] = {
    0x54, 0x4D, 0x47, 0x31, // 'TMG1'
    0x02,                   // Version 2
    0x01,                   // Flags (MsbFirst)
    0x80, 0x00,             // Width  128
    0x40, 0x00,             // Height 64
    0x01, 0x00,             // TimebaseNum 1
    0x1E, 0x00,             // TimebaseDen 30
    0x3C, 0x00              // KeyInterval 60
};

void test_readFileHeader_success() {
    Tmg1Stream stream = makeReadStream(validHeader, sizeof(validHeader));
    tmg1::Decoder dec;
    // ヘッダのみ読んでフレームがないのでReadError になるが、ヘッダは正常に読めた後のエラー
    // begin() がヘッダを読んだ後にフレームデータなしで失敗することを確認
    // ここでは getWidth/Height が正しくセットされることを確認する
    dec.begin(stream); // エラーは無視 (ヘッダ後のフレームデータなし)
    TEST_ASSERT_EQUAL_UINT16(128, dec.getWidth());
    TEST_ASSERT_EQUAL_UINT16(64,  dec.getHeight());
}

void test_readFileHeader_invalid_signature() {
    uint8_t bad[] = {0x58,0x58,0x58,0x58,0x02,0x01,0x80,0x00,0x40,0x00,0x01,0x00,0x1E,0x00,0x3C,0x00};
    Tmg1Stream stream = makeReadStream(bad, sizeof(bad));
    tmg1::Decoder dec;
    tmg1::Error err = dec.begin(stream);
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::InvalidSignature, (int)err);
}

void test_readFileHeader_invalid_version() {
    // version = 1 は v2 デコーダでは InvalidVersion
    uint8_t bad[] = {0x54,0x4D,0x47,0x31,0x01,0x01,0x80,0x00,0x40,0x00,0x01,0x00,0x1E,0x00,0x3C,0x00};
    Tmg1Stream stream = makeReadStream(bad, sizeof(bad));
    tmg1::Decoder dec;
    tmg1::Error err = dec.begin(stream);
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::InvalidVersion, (int)err);
}

void test_readFileHeader_read_error() {
    // ヘッダが短すぎる
    Tmg1Stream stream = makeReadStream(validHeader, sizeof(validHeader) - 1);
    tmg1::Decoder dec;
    tmg1::Error err = dec.begin(stream);
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::ReadError, (int)err);
}

// --- ラウンドトリップテスト (エンコーダ → デコーダ) ---

void test_decode_simple_frames() {
    const uint16_t W = 8, H = 8;
    const size_t frameSize = (W * H + 7) / 8; // 8バイト

    uint8_t frame0[8] = { 0xAA, 0x55, 0xFF, 0x00, 0x0F, 0xF0, 0xCC, 0x33 };
    uint8_t frame1[8] = {};

    // エンコード
    Tmg1MemWriteCtx encCtx = { gEncBuf, sizeof(gEncBuf), 0 };
    Tmg1Stream encStream = { &encCtx, nullptr, tmg1_mem_write, nullptr, nullptr };
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
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.begin(encStream));
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.encodeFrame(frame0, frameSize));
        TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)enc.encodeFrame(frame1, frameSize));
        enc.finish();
    }

    // デコード
    Tmg1Stream decStream = makeReadStream(gEncBuf, encCtx.pos);
    tmg1::Decoder dec;
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.begin(decStream));
    TEST_ASSERT_EQUAL_UINT16(W, dec.getWidth());
    TEST_ASSERT_EQUAL_UINT16(H, dec.getHeight());

    uint8_t out[8];
    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.decodeFrame(out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(frame0, out, frameSize);

    TEST_ASSERT_EQUAL_INT((int)tmg1::Error::None, (int)dec.decodeFrame(out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(frame1, out, frameSize);
}

// ファイルからの読み込みテスト (v2テストデータ未作成のためスキップ)
void test_decode_file_from_simple_tmg1() {
    TEST_IGNORE_MESSAGE("v2 test data file not yet generated. Use encoder to create.");
}
