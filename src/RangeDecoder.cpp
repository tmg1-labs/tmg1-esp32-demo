#include "RangeDecoder.h"

RangeDecoder::RangeDecoder(const uint8_t* data, size_t dataSize, FrequencyModel& model)
    : _data(data), _dataSize(dataSize), _model(model), _byteIndex(0),
      _low(0), _range(0xFFFFFFFFFFFFFFFFULL), _code(0), _context(0) {
    
    // Initialize decoder state
    // The C# code reads 8 bytes.
    for (int i = 0; i < 8; ++i) {
        _code = (_code << 8) | readByte();
    }
}

int RangeDecoder::readBit() {
    return (int)readSymbol();
}

uint32_t RangeDecoder::readBits(int count) {
    uint32_t value = 0;
    for (int i = 0; i < count; ++i) {
        value = (value << 1) | (uint32_t)readBit();
    }
    return value;
}

// Helper for 128-bit arithmetic
#if defined(__SIZEOF_INT128__)
    __extension__ typedef unsigned __int128 uint128_t;
    #define HAS_INT128
#endif

uint32_t RangeDecoder::readSymbol() {
    uint32_t context = _context;
    uint32_t total = _model.getTotalFrequencies(context);

#ifdef HAS_INT128
    // scaledCode = ((_code - _low + 1) * total - 1) / _range;
    // Use uint128_t to prevent overflow
    uint128_t diff = _code - _low + 1;
    uint128_t numerator = diff * total - 1;
    uint128_t scaledCode128 = numerator / _range;
    uint32_t scaledCode = (uint32_t)scaledCode128;

    uint32_t symbolFreq;
    uint32_t symbol = _model.getSymbolFromCumulativeFrequency(scaledCode, context, symbolFreq);
    uint32_t cumulative = _model.getCumulativeFrequency(symbol, context);

    // _low = _low + cumulative * _range / total
    // _range = symbolFreq * _range / total
    uint128_t range128 = _range;
    
    uint128_t newLowOffset = (uint128_t)cumulative * range128 / total;
    _low += (uint64_t)newLowOffset;
    
    uint128_t newRange128 = (uint128_t)symbolFreq * range128 / total;
    _range = (uint64_t)newRange128;
#else
    // Fallback for environments without __int128 support
    // This will likely result in incorrect decoding, but allows compilation.
    // TODO: Implement 64-bit emulation for 128-bit arithmetic if needed for strict 32-bit platforms.
    uint32_t scaledCode = 0;
    uint32_t symbolFreq = 0;
    uint32_t symbol = 0;
    uint32_t cumulative = 0;
#endif

    if (_range == 0) _range = 1;

    _model.update(symbol, context);

    if (_model.getNumContexts() > 1) {
        _context = symbol;
    }

    normalize();
    return symbol;
}

void RangeDecoder::normalize() {
    // TopValue = 1 << 56
    // Check if high bytes match or range is too small
    while ((_low ^ (_low + _range)) < TopValue || _range < BottomValue) {
        _code = (_code << 8) | readByte();
        _low <<= 8;
        _range <<= 8;
    }
}

uint8_t RangeDecoder::readByte() {
    if (_byteIndex < _dataSize) {
        return _data[_byteIndex++];
    }
    return 0;
}

bool RangeDecoder::isEndOfStream() const {
    return _byteIndex >= _dataSize && _range == 0;
}
