#pragma once

#include <Arduino.h>

#include <vector>

#include "FrequencyModel.h"

// エラーコード
enum class Tmg1DecoderError {
  None,
  InvalidSignature,
  ReadError,
  InvalidVersion,
};

// ファイルヘッダ
#pragma pack(push, 1)
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
#pragma pack(pop)

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

  void setReverseBitOrder(bool reverse);

  uint16_t getWidth() const;
  uint16_t getHeight() const;
  uint16_t getTimebaseNum() const;
  uint16_t getTimebaseDen() const;
  uint32_t getLastPtsDelta() const;

 protected:
  Tmg1DecoderError readFileHeader();
  Tmg1DecoderError readFrameHeader(Tmg1FrameHeader& header);
  Tmg1DecoderError readUleb128(uint32_t& value);

  bool decompressPayload(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags,
                         uint8_t frameType);
  bool decompressPayloadRice(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags,
                             uint8_t frameType);
  bool decompressPayloadRange(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags,
                              uint8_t frameType);
  void applyInversePrediction(uint8_t* buffer, size_t bufferSize, uint8_t predictionMethod);
  void applyBitReversal(uint8_t* buffer, size_t bufferSize);

  Stream* _stream = nullptr;
  Tmg1FileHeader _fileHeader;
  uint8_t* _previousFrame = nullptr;
  size_t _frameBufferSize = 0;
  uint32_t _lastPtsDelta = 0;
  bool _reverseBitOrder = false;

  static uint8_t _bitReverseTable[256];
  static bool _isTableInitialized;
  static void initBitReverseTable();

  // Reusable buffers and models
  std::vector<uint8_t> _payloadBuffer;
  uint8_t* _tempFrame = nullptr;  // For P-Frame delta decompression
  size_t _tempFrameSize = 0;
  FrequencyModel _rangeModel;  // Reused frequency model
};