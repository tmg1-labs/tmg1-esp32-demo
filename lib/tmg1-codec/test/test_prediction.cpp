#include <unity.h>
#include <stdint.h>
#include <string.h>
#include "tmg1/prediction.h"
#include "tmg1/types.h"

// 8x1 ピクセル (1行, 1バイト) のラウンドトリップ
void test_prediction_none_identity() {
    uint8_t buf[]  = { 0xAB };
    uint8_t orig[] = { 0xAB };
    tmg1::applyPrediction       (buf, 1, 8, 1, tmg1::PREDICTION_NONE);
    tmg1::applyInversePrediction(buf, 1, 8, 1, tmg1::PREDICTION_NONE);
    TEST_ASSERT_EQUAL_UINT8(orig[0], buf[0]);
}

// 16x1 ピクセル (1行, 2バイト) の左方向
void test_prediction_left_roundtrip() {
    uint8_t buf[]  = { 0b10110011, 0b01011010 };
    uint8_t orig[] = { 0b10110011, 0b01011010 };

    tmg1::applyPrediction       (buf, 2, 16, 1, tmg1::PREDICTION_LEFT);
    // apply後、元の値と異なるはず
    TEST_ASSERT_NOT_EQUAL(orig[1], buf[1]); // 2バイト目は変わる
    tmg1::applyInversePrediction(buf, 2, 16, 1, tmg1::PREDICTION_LEFT);
    TEST_ASSERT_EQUAL_MEMORY(orig, buf, 2);
}

// 8x2 ピクセル (2行, 1バイトずつ) の上方向
void test_prediction_up_roundtrip() {
    uint8_t buf[]  = { 0b11001010, 0b10110101 };
    uint8_t orig[] = { 0b11001010, 0b10110101 };

    tmg1::applyPrediction       (buf, 2, 8, 2, tmg1::PREDICTION_UP);
    TEST_ASSERT_NOT_EQUAL(orig[1], buf[1]);
    tmg1::applyInversePrediction(buf, 2, 8, 2, tmg1::PREDICTION_UP);
    TEST_ASSERT_EQUAL_MEMORY(orig, buf, 2);
}

// 全ゼロのバッファは予測前後も変わらない
void test_prediction_left_all_zero() {
    uint8_t buf[4] = {};
    tmg1::applyPrediction       (buf, 4, 32, 1, tmg1::PREDICTION_LEFT);
    for (int i = 0; i < 4; ++i) TEST_ASSERT_EQUAL_UINT8(0, buf[i]);
    tmg1::applyInversePrediction(buf, 4, 32, 1, tmg1::PREDICTION_LEFT);
    for (int i = 0; i < 4; ++i) TEST_ASSERT_EQUAL_UINT8(0, buf[i]);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_prediction_none_identity);
    RUN_TEST(test_prediction_left_roundtrip);
    RUN_TEST(test_prediction_up_roundtrip);
    RUN_TEST(test_prediction_left_all_zero);
    return UNITY_END();
}
