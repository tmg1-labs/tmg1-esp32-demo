#include <unity.h>

// Test function declarations from test_Tmg1Decoder.cpp
extern void test_Tmg1Decoder_instance_creation();
extern void test_readFileHeader_success();
extern void test_readFileHeader_invalid_signature();
extern void test_readFileHeader_invalid_version();
extern void test_readFileHeader_read_error();
extern void test_decode_simple_frames();
extern void test_decode_file_from_simple_tmg1(); // Declare the new test function

// setUp and tearDown are defined in test_Tmg1Decoder.cpp
extern void setUp(void);
extern void tearDown(void);

// The main function for the native test environment
int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_Tmg1Decoder_instance_creation);
    RUN_TEST(test_readFileHeader_success);
    RUN_TEST(test_readFileHeader_invalid_signature);
    RUN_TEST(test_readFileHeader_invalid_version);
    RUN_TEST(test_readFileHeader_read_error);
    RUN_TEST(test_decode_simple_frames);
    RUN_TEST(test_decode_file_from_simple_tmg1); // Run the new test function

    return UNITY_END();
}
