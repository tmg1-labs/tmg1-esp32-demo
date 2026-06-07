#include <unity.h>
#include <stdint.h>
#include <string.h>
#include <cstdio>
#include "tmg1/freq_model.h"
#include "tmg1/range_decoder.h"
#include "tmg1/range_encoder.h"
#include "tmg1/io.h"

static uint8_t gRangeBuf[1024];

static void setupRangeWriter(Tmg1MemWriteCtx& ctx, Tmg1Stream& stream) {
    memset(gRangeBuf, 0, sizeof(gRangeBuf));
    ctx    = { gRangeBuf, sizeof(gRangeBuf), 0 };
    stream = { &ctx, nullptr, tmg1_mem_write, nullptr, nullptr };
}

// バイナリシンボル列のラウンドトリップ (コンテキスト2)
void test_range_binary_roundtrip() {
    const uint32_t symbols[] = { 0,1,0,0,1,1,0,1,0,0,0,1,1,1,0 };
    const int N = sizeof(symbols) / sizeof(symbols[0]);

    Tmg1MemWriteCtx ctx; Tmg1Stream stream;
    setupRangeWriter(ctx, stream);

    FrequencyModel encModel(2, 2);
    {
        RangeEncoder enc(encModel, stream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(symbols[i]);
        enc.flush();
    }

    FrequencyModel decModel(2, 2);
    RangeDecoder dec(gRangeBuf, ctx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = dec.readSymbol();
        TEST_ASSERT_EQUAL_UINT32(symbols[i], sym);
    }
}

// 長いシーケンス (コンテキスト1)
void test_range_long_sequence() {
    const int N = 72;
    uint32_t symbols[N];
    for (int i = 0; i < N; ++i) symbols[i] = i % 2;

    Tmg1MemWriteCtx ctx; Tmg1Stream stream;
    setupRangeWriter(ctx, stream);

    FrequencyModel encModel(2, 1);
    {
        RangeEncoder enc(encModel, stream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(symbols[i]);
        enc.flush();
    }

    FrequencyModel decModel(2, 1);
    RangeDecoder dec(gRangeBuf, ctx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = dec.readSymbol();
        if (sym != symbols[i]) {
            char msg[80];
            snprintf(msg, sizeof(msg), "idx=%d exp=%u got=%u", i, symbols[i], sym);
            TEST_FAIL_MESSAGE(msg);
            return;
        }
    }
}

// 全0シーケンス
void test_range_all_zeros() {
    const int N = 50;
    Tmg1MemWriteCtx ctx; Tmg1Stream stream;
    setupRangeWriter(ctx, stream);

    FrequencyModel encModel(2, 1);
    {
        RangeEncoder enc(encModel, stream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(0);
        enc.flush();
    }

    FrequencyModel decModel(2, 1);
    RangeDecoder dec(gRangeBuf, ctx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        TEST_ASSERT_EQUAL_UINT32(0u, dec.readSymbol());
    }
}

// 全1シーケンス
void test_range_all_ones() {
    const int N = 50;
    Tmg1MemWriteCtx ctx; Tmg1Stream stream;
    setupRangeWriter(ctx, stream);

    FrequencyModel encModel(2, 1);
    {
        RangeEncoder enc(encModel, stream);
        for (int i = 0; i < N; ++i) enc.writeSymbol(1);
        enc.flush();
    }

    FrequencyModel decModel(2, 1);
    RangeDecoder dec(gRangeBuf, ctx.pos, decModel);
    for (int i = 0; i < N; ++i) {
        TEST_ASSERT_EQUAL_UINT32(1u, dec.readSymbol());
    }
}
