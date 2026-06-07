#include <unity.h>
#include <stdint.h>
#include <string.h>
#include "tmg1/rice_reader.h"
#include "tmg1/rice_writer.h"
#include "tmg1/io.h"

// ライタ用出力バッファ
static uint8_t gBuf[256];
static Tmg1MemWriteCtx gWriteCtx;
static Tmg1Stream      gWriteStream;

static void setupWriter() {
    memset(gBuf, 0, sizeof(gBuf));
    gWriteCtx    = { gBuf, sizeof(gBuf), 0 };
    gWriteStream = { &gWriteCtx, nullptr, tmg1_mem_write, nullptr, nullptr };
}

// RiceBitWriter で書いた後、RiceBitReader で読んで一致を確認するヘルパー
static void roundtripBits(const uint8_t* bits, int count, bool msbFirst) {
    setupWriter();
    RiceBitWriter writer(gWriteStream, msbFirst);
    for (int i = 0; i < count; ++i) writer.writeBit(bits[i]);
    writer.flush();

    RiceBitReader reader(gBuf, gWriteCtx.pos, msbFirst);
    for (int i = 0; i < count; ++i) {
        int bit = reader.readBit();
        TEST_ASSERT_EQUAL_INT(bits[i], bit);
    }
}

void test_rice_single_bit_msb() {
    uint8_t bits[] = { 1, 0, 1, 1, 0 };
    roundtripBits(bits, 5, true);
}

void test_rice_single_bit_lsb() {
    uint8_t bits[] = { 0, 1, 1, 0, 0, 1 };
    roundtripBits(bits, 6, false);
}

void test_rice_symbol_roundtrip_k1() {
    const uint32_t symbols[] = { 0, 1, 2, 3, 7, 15, 31 };
    const int N = sizeof(symbols) / sizeof(symbols[0]);

    setupWriter();
    RiceBitWriter writer(gWriteStream, true);
    for (int i = 0; i < N; ++i) writer.writeSymbol(symbols[i], 1);
    writer.flush();

    RiceBitReader reader(gBuf, gWriteCtx.pos, true);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = reader.readSymbol(1);
        TEST_ASSERT_EQUAL_UINT32(symbols[i], sym);
    }
}

void test_rice_symbol_roundtrip_k3() {
    const uint32_t symbols[] = { 0, 1, 7, 8, 15, 63 };
    const int N = sizeof(symbols) / sizeof(symbols[0]);

    setupWriter();
    RiceBitWriter writer(gWriteStream, true);
    for (int i = 0; i < N; ++i) writer.writeSymbol(symbols[i], 3);
    writer.flush();

    RiceBitReader reader(gBuf, gWriteCtx.pos, true);
    for (int i = 0; i < N; ++i) {
        uint32_t sym = reader.readSymbol(3);
        TEST_ASSERT_EQUAL_UINT32(symbols[i], sym);
    }
}

void test_rice_eos() {
    setupWriter();
    RiceBitWriter writer(gWriteStream, true);
    writer.flush();
    RiceBitReader reader(gBuf, 0, true);
    TEST_ASSERT_TRUE(reader.isEndOfStream());
    TEST_ASSERT_EQUAL_INT(-1, reader.readBit());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_rice_single_bit_msb);
    RUN_TEST(test_rice_single_bit_lsb);
    RUN_TEST(test_rice_symbol_roundtrip_k1);
    RUN_TEST(test_rice_symbol_roundtrip_k3);
    RUN_TEST(test_rice_eos);
    return UNITY_END();
}
