#include <unity.h>

// test_Tmg1Decoder.cpp で定義
extern void test_Tmg1Decoder_instance_creation();
extern void test_readFileHeader_success();
extern void test_readFileHeader_invalid_signature();
extern void test_readFileHeader_invalid_version();
extern void test_readFileHeader_read_error();
extern void test_decode_simple_frames();
extern void test_decode_file_from_simple_tmg1();

// test_range_coder.cpp で定義
extern void test_range_binary_roundtrip();
extern void test_range_long_sequence();
extern void test_range_all_zeros();
extern void test_range_all_ones();

extern void setUp(void);
extern void tearDown(void);

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_range_binary_roundtrip);
    RUN_TEST(test_range_long_sequence);
    RUN_TEST(test_range_all_zeros);
    RUN_TEST(test_range_all_ones);

    RUN_TEST(test_Tmg1Decoder_instance_creation);
    RUN_TEST(test_readFileHeader_success);
    RUN_TEST(test_readFileHeader_invalid_signature);
    RUN_TEST(test_readFileHeader_invalid_version);
    RUN_TEST(test_readFileHeader_read_error);
    RUN_TEST(test_decode_simple_frames);
    RUN_TEST(test_decode_file_from_simple_tmg1);

    return UNITY_END();
}
