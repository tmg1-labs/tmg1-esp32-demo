#include <unity.h>
#include <stdint.h>
#include <string.h>
#include "tmg1/freq_model.h"
#include "tmg1/range_decoder.h"
#include "tmg1/range_encoder.h"
#include "tmg1/io.h"

static uint8_t gBuf[1024];
static Tmg1MemWriteCtx gWriteCtx;
static Tmg1Stream      gWriteStream;

static void setupWriter() {
    memset(gBuf, 0, sizeof(gBuf));
    gWriteCtx    = { gBuf, sizeof(gBuf), 0 };
    gWriteStream = { &gWriteCtx, nullptr, tmg1_mem_write, nullptr, nullptr };
}

// バイナリシンボル列のラウンドトリップ
void test_range_binary_roundtrip() {
    const uint32_t symbols[] = { 0,1,0,0,1,1,0,1,0,0,0,1,1,1,0 };
    const int N = sizeof(symbols) / sizeof(symbols[0]);

    setupWriter();
    FrequencyModel encModel(2, 2);
    {
        RangeEncoder enc(encModel, gWriteStream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(symbols[i]);
        enc.flush();
    }

    FrequencyModel decModel(2, 2);
    RangeDecoder dec(gBuf, gWriteCtx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = dec.readSymbol();
        TEST_ASSERT_EQUAL_UINT32(symbols[i], sym);
    }
}

// 長いシーケンスで統計的な安定性を確認
void test_range_long_sequence() {
    const int N = 200;
    uint32_t symbols[N];
    for (int i = 0; i < N; ++i) symbols[i] = i % 2;

    setupWriter();
    FrequencyModel encModel(2, 1);
    {
        RangeEncoder enc(encModel, gWriteStream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(symbols[i]);
        enc.flush();
    }

    FrequencyModel decModel(2, 1);
    RangeDecoder dec(gBuf, gWriteCtx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = dec.readSymbol();
        TEST_ASSERT_EQUAL_UINT32(symbols[i], sym);
    }
}

// 全0シーケンス (エッジケース)
void test_range_all_zeros() {
    const int N = 50;

    setupWriter();
    FrequencyModel encModel(2, 1);
    {
        RangeEncoder enc(encModel, gWriteStream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(0);
        enc.flush();
    }

    FrequencyModel decModel(2, 1);
    RangeDecoder dec(gBuf, gWriteCtx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        TEST_ASSERT_EQUAL_UINT32(0u, dec.readSymbol());
    }
}

// 全1シーケンス (エッジケース)
void test_range_all_ones() {
    const int N = 50;

    setupWriter();
    FrequencyModel encModel(2, 1);
    {
        RangeEncoder enc(encModel, gWriteStream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(1);
        enc.flush();
    }

    FrequencyModel decModel(2, 1);
    RangeDecoder dec(gBuf, gWriteCtx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        TEST_ASSERT_EQUAL_UINT32(1u, dec.readSymbol());
    }
}

// コンテキストモデル (numContexts=2) のラウンドトリップ
void test_range_context_model() {
    const uint32_t symbols[] = { 0,1,0,1,0,0,1,1,0,1 };
    const int N = sizeof(symbols) / sizeof(symbols[0]);

    setupWriter();
    FrequencyModel encModel(2, 2);
    {
        RangeEncoder enc(encModel, gWriteStream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(symbols[i]);
        enc.flush();
    }

    FrequencyModel decModel(2, 2);
    RangeDecoder dec(gBuf, gWriteCtx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = dec.readSymbol();
        TEST_ASSERT_EQUAL_UINT32(symbols[i], sym);
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_range_binary_roundtrip);
    RUN_TEST(test_range_long_sequence);
    RUN_TEST(test_range_all_zeros);
    RUN_TEST(test_range_all_ones);
    RUN_TEST(test_range_context_model);
    return UNITY_END();
}
