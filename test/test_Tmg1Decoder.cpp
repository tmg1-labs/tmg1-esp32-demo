#include <unity.h>
#include <fstream> // Required for file operations
#include <vector>  // Required for std::vector
#include <string.h> // Required for memset

#include "Tmg1Decoder.h"
#include "MemoryStream.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

// This test may fail until begin is implemented
void test_Tmg1Decoder_begin_false_with_null_stream() {
    Tmg1Decoder decoder;
    // It is not possible to create a Stream instance directly, so we pass nullptr.
    // The begin method should handle this gracefully.
    // TEST_ASSERT_FALSE(decoder.begin(*(Stream*)nullptr));
    // Note: Dereferencing a null pointer is undefined behavior.
    // A better way to test this would be to pass a valid but empty stream if needed,
    // but the decoder should robustly handle stream read failures.
    // For now, we'll focus on tests with MemoryStream.
}

void test_Tmg1Decoder_instance_creation() {
    Tmg1Decoder decoder;
    TEST_ASSERT_NOT_NULL(&decoder);
    TEST_ASSERT_EQUAL(0, decoder.getWidth());
    TEST_ASSERT_EQUAL(0, decoder.getHeight());
}

const uint8_t validHeader[] = {
    0x54, 0x4D, 0x47, 0x31, // 'TMG1'
    0x01,                   // Version 1
    0x01,                   // Flags (MsbFirst)
    0x80, 0x00,             // Width 128
    0x40, 0x00,             // Height 64
    0x01, 0x00,             // TimebaseNum 1
    0x1E, 0x00,             // TimebaseDen 30
    0x3C, 0x00              // KeyInterval 60
};

void test_readFileHeader_success() {
    MemoryStream stream(validHeader, sizeof(validHeader));
    Tmg1Decoder decoder;
    TEST_ASSERT_TRUE(decoder.begin(stream));
    TEST_ASSERT_EQUAL(128, decoder.getWidth());
    TEST_ASSERT_EQUAL(64, decoder.getHeight());
}

void test_readFileHeader_invalid_signature() {
    uint8_t invalidSignatureHeader[] = { 0x58, 0x58, 0x58, 0x58, 0x01, 0x01, 0x80, 0x00, 0x40, 0x00, 0x01, 0x00, 0x1E, 0x00, 0x3C, 0x00 };
    MemoryStream stream(invalidSignatureHeader, sizeof(invalidSignatureHeader));
    Tmg1Decoder decoder;
    TEST_ASSERT_FALSE(decoder.begin(stream));
}

void test_readFileHeader_invalid_version() {
    uint8_t invalidVersionHeader[] = { 0x54, 0x4D, 0x47, 0x31, 0x02, 0x01, 0x80, 0x00, 0x40, 0x00, 0x01, 0x00, 0x1E, 0x00, 0x3C, 0x00 };
    MemoryStream stream(invalidVersionHeader, sizeof(invalidVersionHeader));
    Tmg1Decoder decoder;
    TEST_ASSERT_FALSE(decoder.begin(stream));
}

void test_readFileHeader_read_error() {
    // Header is truncated
    MemoryStream stream(validHeader, sizeof(validHeader) - 1);
    Tmg1Decoder decoder;
    TEST_ASSERT_FALSE(decoder.begin(stream));
}

unsigned char test_data_simple_tmg1_embedded[] = { 0x54, 0x4D, 0x47, 0x31, 0x01, 0x01, 0x80, 0x00, 0x40, 0x00, 0x01, 0x00, 0x1E, 0x00, 0x3C, 0x00, 0x00, 0x01, 0x68, 0x02, 0x00, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0xEC, 0x07, 0x60, 0x3B, 0x01, 0xD8, 0x0E, 0xC0, 0x76, 0x03, 0xB0, 0x1D, 0x80, 0x01, 0x01, 0x0B, 0x02, 0x02, 0xD0, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned int test_data_simple_tmg1_embedded_len = 141;

void test_decode_simple_frames() {
    // Arrange
    const int width = 128;
    const int height = 64;
    const int bytesPerFrame = (width / 8) * height;

    uint8_t decodedFrame[bytesPerFrame];
    
    uint8_t blackFrame[bytesPerFrame];
    memset(blackFrame, 0x00, sizeof(blackFrame));

    uint8_t whiteFrame[bytesPerFrame];
    memset(whiteFrame, 0xFF, sizeof(whiteFrame));

    MemoryStream stream(test_data_simple_tmg1_embedded, test_data_simple_tmg1_embedded_len);
    Tmg1Decoder decoder;

    // Act & Assert
    TEST_ASSERT_TRUE(decoder.begin(stream));
    TEST_ASSERT_EQUAL(width, decoder.getWidth());
    TEST_ASSERT_EQUAL(height, decoder.getHeight());

    // Frame 1 (I-Frame, black)
    TEST_ASSERT_TRUE(decoder.decodeFrame(decodedFrame, sizeof(decodedFrame)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(blackFrame, decodedFrame, bytesPerFrame);

    // Frame 2 (P-Frame, white)
    TEST_ASSERT_TRUE(decoder.decodeFrame(decodedFrame, sizeof(decodedFrame)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(whiteFrame, decodedFrame, bytesPerFrame);
}

void test_decode_file_from_simple_tmg1() {
    // Arrange
    const int width = 128;
    const int height = 64;
    const int bytesPerFrame = (width / 8) * height;

    uint8_t decodedFrame[bytesPerFrame];
    
    uint8_t blackFrame[bytesPerFrame];
    memset(blackFrame, 0x00, sizeof(blackFrame));

    uint8_t whiteFrame[bytesPerFrame];
    memset(whiteFrame, 0xFF, sizeof(whiteFrame));

    // --- File Reading Logic ---
    // The path to the test data file. Assuming it's in the test directory.
    const char* filePath = "test/test_data_simple.tmg1";
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    
    TEST_ASSERT_TRUE(file.is_open()); // Assert that the file opened successfully
    if (!file.is_open()) {
        TEST_FAIL_MESSAGE("Failed to open test_data_simple.tmg1");
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    TEST_ASSERT_TRUE(file.good()); // Assert read success
    if (!file.good()) {
        TEST_FAIL_MESSAGE("Failed to read test_data_simple.tmg1");
        file.close();
        return;
    }
    file.close();

    MemoryStream stream(buffer.data(), buffer.size());
    Tmg1Decoder decoder;

    // Act & Assert
    TEST_ASSERT_TRUE(decoder.begin(stream));
    TEST_ASSERT_EQUAL(width, decoder.getWidth());
    TEST_ASSERT_EQUAL(height, decoder.getHeight());

    // Frame 1 (I-Frame, black)
    TEST_ASSERT_TRUE(decoder.decodeFrame(decodedFrame, sizeof(decodedFrame)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(blackFrame, decodedFrame, bytesPerFrame);

    // Frame 2 (P-Frame, white)
    TEST_ASSERT_TRUE(decoder.decodeFrame(decodedFrame, sizeof(decodedFrame)));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(whiteFrame, decodedFrame, bytesPerFrame);
}