#include "RiceBitReader.h"

RiceBitReader::RiceBitReader(const uint8_t* src, size_t srcSize, bool msbFirst)
    : _src(src), _srcSize(srcSize), _msbFirst(msbFirst), _byteIndex(0), _bitIndex(0) {}

int RiceBitReader::readBit() {
    if (isEndOfStream()) {
        return -1; // End of stream
    }

    uint8_t currentByte = _src[_byteIndex];
    int bit;

    if (_msbFirst) {
        bit = (currentByte >> (7 - _bitIndex)) & 1;
    } else {
        bit = (currentByte >> _bitIndex) & 1;
    }

    _bitIndex++;
    if (_bitIndex == 8) {
        _bitIndex = 0;
        _byteIndex++;
    }

    return bit;
}

uint32_t RiceBitReader::readBits(int count) {
    uint32_t value = 0;
    for (int i = 0; i < count; ++i) {
        int bit = readBit();
        if (bit == -1) {
            return 0xFFFFFFFF; // Indicate error/EOS
        }
        // The encoder writes bits from MSB to LSB for multi-bit values,
        // so we shift and OR regardless of the byte-level bit order (_msbFirst).
        value = (value << 1) | bit;
    }
    return value;
}

uint32_t RiceBitReader::readSymbol(int parameter) {
    // Decode unary part (quotient)
    uint32_t q = 0;
    while (readBit() == 1) {
        q++;
    }

    // Decode binary part (remainder)
    uint32_t r = readBits(parameter);

    if (r == 0xFFFFFFFF) {
        return 0xFFFFFFFF; // Propagate error
    }

    // Reconstruct symbol
    return (q << parameter) | r;
}

bool RiceBitReader::isEndOfStream() const {
    return _byteIndex >= _srcSize;
}
