#pragma once
#include <Arduino.h>

class RiceBitReader {
public:
    RiceBitReader(const uint8_t* src, size_t srcSize, bool msbFirst);
    int readBit();
    uint32_t readBits(int count);
    uint32_t readSymbol(int parameter);
    bool isEndOfStream() const;

private:
    const uint8_t* _src;
    size_t _srcSize;
    bool _msbFirst;
    size_t _byteIndex;
    uint8_t _bitIndex; // 0-7
};
