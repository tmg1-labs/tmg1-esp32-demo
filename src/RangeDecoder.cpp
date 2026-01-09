#include "RangeDecoder.h"

// --- Helper for 128-bit arithmetic on 32-bit platforms ---

struct uint128_val {
    uint64_t lo;
    uint64_t hi;
};

// 64x64 -> 128 multiplication
static uint128_val mul64(uint64_t u, uint64_t v) {
    uint64_t u0 = u & 0xFFFFFFFF;
    uint64_t u1 = u >> 32;
    uint64_t v0 = v & 0xFFFFFFFF;
    uint64_t v1 = v >> 32;

    uint64_t t = u0 * v0;
    uint64_t w0 = t & 0xFFFFFFFF;
    uint64_t k = t >> 32;

    t = u1 * v0 + k;
    uint64_t w1 = t & 0xFFFFFFFF;
    uint64_t w2 = t >> 32;

    t = u0 * v1 + w1;
    k = t >> 32;

    uint64_t lo = (t << 32) + w0;
    uint64_t hi = u1 * v1 + w2 + k;

    return {lo, hi};
}

// 128 / 64 division
// Returns the quotient (lo part only, assuming result fits in 64 bits)
static uint64_t div128by64(uint128_val u, uint64_t v) {
    if (v == 0) return 0;
    
    // If high part is 0, use standard 64-bit division
    if (u.hi == 0) {
        return u.lo / v;
    }
    
    // Bitwise long division for 128-bit dividend and 64-bit divisor
    uint128_val rem = {0, 0};
    uint64_t quot = 0;
    
    // Iterate from bit 127 down to 0
    for (int i = 127; i >= 0; i--) {
        // rem <<= 1
        uint64_t rem_carry = rem.lo >> 63;
        rem.lo = (rem.lo << 1);
        rem.hi = (rem.hi << 1) | rem_carry;
        
        // rem |= (u bit i)
        uint64_t bit;
        if (i >= 64) bit = (u.hi >> (i - 64)) & 1;
        else bit = (u.lo >> i) & 1;
        
        rem.lo |= bit;
        
        // Check if rem >= v
        // Since v is 64-bit, if rem.hi > 0 then rem is definitely > v
        bool ge = (rem.hi > 0) || (rem.lo >= v);
        
        if (ge) {
            // rem -= v
            if (rem.lo < v) rem.hi--; // borrow
            rem.lo -= v;
            
            // set bit i of quotient
            if (i < 64) quot |= (1ULL << i);
        }
    }
    return quot;
}

// 128-bit addition (a + b)
static uint128_val add128(uint128_val a, uint128_val b) {
    uint128_val res;
    res.lo = a.lo + b.lo;
    res.hi = a.hi + b.hi + (res.lo < a.lo ? 1 : 0);
    return res;
}

// 128-bit subtraction (a - b)
static uint128_val sub128(uint128_val a, uint64_t b) {
    uint128_val res = a;
    if (res.lo < b) res.hi--;
    res.lo -= b;
    return res;
}

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

uint32_t RangeDecoder::readSymbol() {
    uint32_t context = _context;
    uint32_t total = _model.getTotalFrequencies(context);

    // C# Logic:
    // var scaledCode = (new BigInteger(_code - _low + 1) * total - 1) / new BigInteger(_range);

    // 1. diff = _code - _low + 1
    // Note: _code and _low are uint64_t. We treat them as if they are part of infinite precision,
    // but in practice standard wrapping arithmetic works for the difference if we assume standard 2's complement,
    // however, for Range Coder, strict value relation is expected.
    // _code is always within [_low, _low + _range).
    uint64_t diff64 = _code - _low + 1;
    
    // 2. prod = diff * total
    uint128_val prod = mul64(diff64, (uint64_t)total);
    
    // 3. prod = prod - 1
    prod = sub128(prod, 1);
    
    // 4. scaledCode = prod / _range
    uint64_t scaledCode64 = div128by64(prod, _range);
    uint32_t scaledCode = (uint32_t)scaledCode64;

    if (scaledCode >= total) scaledCode = total - 1;

    uint32_t symbolFreq;
    uint32_t symbol = _model.getSymbolFromCumulativeFrequency(scaledCode, context, symbolFreq);
    uint32_t cumulative = _model.getCumulativeFrequency(symbol, context);

    // Update range and low using high precision
    // C# Logic:
    // var newLow = new BigInteger(_low) + cumulative * bigRange / total;
    // var newRange = symbolFreq * bigRange / total;

    // Common term: cumulative * _range
    uint128_val range128_prod_cum = mul64((uint64_t)cumulative, _range);
    uint64_t offset = div128by64(range128_prod_cum, (uint64_t)total);
    
    // newLow = _low + offset
    // Check for overflow? _low is 64bit. Result fits in 64bit by definition of Range Coder.
    uint64_t newLow = _low + offset;

    // newRange calculation
    uint128_val range128_prod_freq = mul64((uint64_t)symbolFreq, _range);
    uint64_t newRange = div128by64(range128_prod_freq, (uint64_t)total);
    
    // Ensure range is at least 1
    if (newRange == 0) newRange = 1;
    
    _low = newLow;
    _range = newRange;

    _model.update(symbol, context);

    // Update context if model supports multiple contexts
    if (_model.getNumContexts() > 1) {
        _context = symbol;
    }

    normalize();
    return symbol;
}

void RangeDecoder::normalize() {
    // Normalize if:
    // 1. Top bytes of low and low+range match (no carry possible) -> ((_low ^ (_low + _range)) < TopValue)
    // 2. Range is too small -> (_range < BottomValue)
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

void RangeDecoder::setContext(uint32_t context) {
    _context = context;
}