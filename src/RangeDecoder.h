#ifndef RANGE_DECODER_H
#define RANGE_DECODER_H

#include <stdint.h>
#include <stddef.h>
#include "FrequencyModel.h"

class RangeDecoder {
public:
    RangeDecoder(const uint8_t* data, size_t dataSize, FrequencyModel& model);
    
    int readBit();
    uint32_t readBits(int count);
    uint32_t readSymbol();
    bool isEndOfStream() const;
    void setContext(uint32_t context);

private:
    void normalize();
    uint8_t readByte();

    const uint8_t* _data;
    size_t _dataSize;
    size_t _byteIndex;
    FrequencyModel& _model;

    uint64_t _low;
    uint64_t _range;
    uint64_t _code;
    uint32_t _context;

    static const uint64_t TopValue = 1ULL << 56; // 1 << (32 + 24)
    static const uint64_t BottomValue = 1ULL << 24;
};

#endif // RANGE_DECODER_H
