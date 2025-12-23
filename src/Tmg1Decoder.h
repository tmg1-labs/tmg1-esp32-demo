#pragma once

#include <Arduino.h>

// エラーコード
enum class Tmg1DecoderError {
    None,
    InvalidSignature,
    ReadError,
    InvalidVersion,
};

// ファイルヘッダ
struct Tmg1FileHeader {
    uint32_t signature;
    uint8_t version;
    uint8_t flags;
    uint16_t width;
    uint16_t height;
    uint16_t timebaseNum;
    uint16_t timebaseDen;
    uint16_t keyInterval;
};

// フレームヘッダ
struct Tmg1FrameHeader {
    uint8_t frameType;
    uint32_t ptsDelta;
    uint32_t payloadSize;
    uint8_t frameFlags;
    uint8_t predictionMethod;
};

class Tmg1Decoder {
public:
    Tmg1Decoder();
    ~Tmg1Decoder();

    bool begin(Stream& stream);
    bool decodeFrame(uint8_t* buffer, size_t bufferSize);
    
    uint16_t getWidth() const;
    uint16_t getHeight() const;

protected:
    Tmg1DecoderError readFileHeader();
    Tmg1DecoderError readFrameHeader(Tmg1FrameHeader& header);
    Tmg1DecoderError readUleb128(uint32_t& value);

    bool decompressPayload(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags, uint8_t frameType);
    bool decompressPayloadRice(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags, uint8_t frameType);
    bool decompressPayloadRange(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags, uint8_t frameType);
    void applyInversePrediction(uint8_t* buffer, size_t bufferSize, uint8_t predictionMethod);

    Stream* _stream = nullptr;
    Tmg1FileHeader _fileHeader;
    uint8_t* _previousFrame = nullptr;
    size_t _frameBufferSize = 0;
};
