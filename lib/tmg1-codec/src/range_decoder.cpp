#include "tmg1/range_decoder.h"

RangeDecoder::RangeDecoder(const uint8_t* data, size_t dataSize, FrequencyModel& model)
    : _data(data), _dataSize(dataSize), _byteIndex(0), _model(model),
      _low(0), _range(0xFFFFFFFFFFFFFFFFull), _code(0), _context(0)
{
    for (int i = 0; i < 8; ++i) {
        _code = (_code << 8) | readByte();
    }
}

IRAM_ATTR int RangeDecoder::readBit() {
    return (int)readSymbol();
}

uint32_t RangeDecoder::readBits(int count) {
    uint32_t value = 0;
    for (int i = 0; i < count; ++i) {
        value = (value << 1) | (uint32_t)readBit();
    }
    return value;
}

IRAM_ATTR uint32_t RangeDecoder::readSymbol() {
    uint32_t context = _context;
    uint32_t total   = _model.getTotalFrequencies(context);

    uint64_t step       = _range / total;
    uint64_t scaledCode = (_code - _low) / step;
    if (scaledCode >= total) scaledCode = total - 1;

    uint32_t symbolFreq;
    uint32_t symbol     = _model.getSymbolFromCumulativeFrequency(
                              (uint32_t)scaledCode, context, symbolFreq);
    uint32_t cumulative = _model.getCumulativeFrequency(symbol, context);

    _low  += step * cumulative;
    _range = step * symbolFreq;
    if (_range == 0) _range = 1;

    _model.update(symbol, context);
    if (_model.getNumContexts() > 1) _context = symbol;

    normalize();
    return symbol;
}

bool RangeDecoder::isEndOfStream() const {
    return _byteIndex >= _dataSize && _range == 0;
}

void RangeDecoder::setContext(uint32_t context) {
    _context = context;
}

IRAM_ATTR void RangeDecoder::normalize() {
    while (true) {
        uint64_t sum      = _low + _range;
        bool     overflow = (sum < _low);
        uint64_t xorVal   = overflow ? 0xFFFFFFFFFFFFFFFFull : (_low ^ sum);
        if (xorVal >= TopValue && _range >= BottomValue) break;
        _code  = (_code  << 8) | readByte();
        _low  <<= 8;
        _range <<= 8;
    }
}

inline uint8_t RangeDecoder::readByte() {
    if (_byteIndex < _dataSize) return _data[_byteIndex++];
    return 0;
}
