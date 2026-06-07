#pragma once
#include <stddef.h>
#include <stdint.h>
#include "freq_model.h"

#if defined(ESP32)
#  include <Arduino.h>
#elif !defined(IRAM_ATTR)
#  define IRAM_ATTR
#endif

// Rangeデコーダ (64bit, __uint128_t不要)
// エンコーダと対になる XOR+BottomValue 正規化 / divide-first 演算
class RangeDecoder {
public:
    static const uint64_t TopValue    = 1ULL << 56;
    static const uint64_t BottomValue = 1ULL << 24;

    RangeDecoder(const uint8_t* data, size_t dataSize, FrequencyModel& model);

    IRAM_ATTR int      readBit();
    uint32_t           readBits(int count);
    IRAM_ATTR uint32_t readSymbol();

    bool     isEndOfStream() const;
    void     setContext(uint32_t context);

private:
    IRAM_ATTR void    normalize();
    inline    uint8_t readByte();

    const uint8_t* _data;
    size_t         _dataSize;
    size_t         _byteIndex;
    FrequencyModel& _model;

    uint64_t _low;
    uint64_t _range;
    uint64_t _code;
    uint32_t _context;
};
