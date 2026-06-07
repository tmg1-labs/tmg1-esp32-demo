#include "tmg1/range_encoder.h"

RangeEncoder::RangeEncoder(FrequencyModel& model, Tmg1Stream& out)
    : _model(model), _out(out),
      _low(0), _range(0xFFFFFFFFFFFFFFFFull), _context(0), _bytesWritten(0)
{}

void RangeEncoder::writeBit(int bit) {
    writeSymbol((uint32_t)bit);
}

void RangeEncoder::writeBits(uint32_t value, int count) {
    for (int i = count - 1; i >= 0; --i) {
        writeBit((int)((value >> i) & 1));
    }
}

void RangeEncoder::writeSymbol(uint32_t symbol) {
    uint32_t context    = _context;
    uint32_t total      = _model.getTotalFrequencies(context);
    uint32_t cumulative = _model.getCumulativeFrequency(symbol, context);
    uint32_t symbolFreq = _model.getSymbolFrequency(symbol, context);

    // divide-first: step*symbolFreq <= step*total <= _range < 2^64 → uint64のみで完結
    uint64_t step = _range / total;
    _low  += step * cumulative;
    _range = step * symbolFreq;
    if (_range == 0) _range = 1;

    _model.update(symbol, context);
    if (_model.getNumContexts() > 1) _context = symbol;

    normalize();
}

int RangeEncoder::flush() {
    for (int i = 0; i < 8; ++i) {
        if (writeByte((uint8_t)(_low >> 56)) < 0) return -1;
        _low <<= 8;
    }
    return 0;
}

void RangeEncoder::normalize() {
    while (true) {
        // _low + _range のオーバーフローを検出してXORを安全に計算
        uint64_t sum      = _low + _range;
        bool     overflow = (sum < _low);
        uint64_t xorVal   = overflow ? 0xFFFFFFFFFFFFFFFFull : (_low ^ sum);
        if (xorVal >= TopValue && _range >= BottomValue) break;
        if (writeByte((uint8_t)(_low >> 56)) < 0) return;
        _low   <<= 8;
        _range <<= 8;
    }
}

int RangeEncoder::writeByte(uint8_t byte) {
    if (_out.write(_out.ctx, &byte, 1) != 1) return -1;
    _bytesWritten++;
    return 0;
}
