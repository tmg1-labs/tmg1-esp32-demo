#ifndef RANGE_DECODER_H
#define RANGE_DECODER_H

#include <stdint.h>
#include <stddef.h>
#include "FrequencyModel.h"

// --- Helper for 128-bit arithmetic on 32-bit platforms ---

struct uint128_val {
    uint64_t lo;
    uint64_t hi;
};

// 64x64 -> 128 multiplication
static inline uint128_val mul64(uint64_t u, uint64_t v) {
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

// 64x32 -> 128 multiplication (optimized)
static inline uint128_val mul64x32(uint64_t u, uint32_t v) {
    uint64_t u0 = u & 0xFFFFFFFF;
    uint64_t u1 = u >> 32;

    uint64_t t = u0 * v;
    uint64_t w0 = t & 0xFFFFFFFF;
    uint64_t k = t >> 32;

    uint64_t w1 = u1 * v + k;

    return { (w1 << 32) | w0, w1 >> 32 };
}

// 128 / 64 division
// Optimized using "Long Division" algorithm (Algorithm D logic) to avoid bit-wise loop.
static inline uint64_t div128by64(uint128_val u, uint64_t v) {
    if (v == 0) return 0;

    // Fast path: if divisor fits in 32 bits
    if (v <= 0xFFFFFFFF) {
        uint32_t v32 = (uint32_t)v;
        uint64_t rem = 0;
        
        rem = (rem << 32) | (u.hi >> 32);
        rem %= v32;
        rem = (rem << 32) | (u.hi & 0xFFFFFFFF);
        rem %= v32;
        
        rem = (rem << 32) | (u.lo >> 32);
        uint64_t q1 = rem / v32;
        rem %= v32;
        
        rem = (rem << 32) | (u.lo & 0xFFFFFFFF);
        uint64_t q0 = rem / v32;
        
        return (q1 << 32) | q0;
    }

    // Fast path: if high part of dividend is 0, standard 64-bit division
    if (u.hi == 0) {
        return u.lo / v;
    }

    // --- General case: 128 / 64 (where result fits in 64 bits) ---
    // Since v > 0xFFFFFFFF, v has at least 33 bits.
    // We normalize divisor v so that its MSB is 1 (bit 63).
    
    int s = __builtin_clzll(v); // count leading zeros
    
    uint64_t v_n = v << s; // normalized divisor
    uint64_t v_hi = v_n >> 32;
    uint64_t v_lo = v_n & 0xFFFFFFFF;

    // Shift u left by s to match v
    uint128_val u_n;
    if (s == 0) {
        u_n = u;
    } else {
        u_n.hi = (u.hi << s) | (u.lo >> (64 - s));
        u_n.lo = u.lo << s;
    }

    // Break u_n into 4 32-bit words: u3, u2, u1, u0
    uint64_t u3 = u_n.hi >> 32;
    uint64_t u2 = u_n.hi & 0xFFFFFFFF;
    uint64_t u1 = u_n.lo >> 32;
    uint64_t u0 = u_n.lo & 0xFFFFFFFF;

    // We compute quotient in two 32-bit chunks: q1 (high), q0 (low)
    
    // --- Compute q1 (High 32 bits) ---
    uint64_t q1 = 0;
    uint64_t dividend_hi = (u3 << 32) | u2;
    
    // Check if dividend_hi is large enough
    if (dividend_hi >= v_hi) {
       uint64_t q_hat = dividend_hi / v_hi;
       uint64_t r_hat = dividend_hi % v_hi;
       
       // Refine q_hat
       while (q_hat > 0xFFFFFFFF || (q_hat * v_lo > ((r_hat << 32) | u1))) {
           q_hat--;
           r_hat += v_hi;
           if (r_hat > 0xFFFFFFFF) break; 
       }
       q1 = q_hat;
    }
    
    // Subtract q1 * v from u (working on top 3 words)
    uint64_t p1 = q1 * v_lo;
    uint64_t p2 = q1 * v_hi;
    
    uint128_val sub_val;
    sub_val.lo = p1 << 32;
    sub_val.hi = p2 + (p1 >> 32);
    
    // u_n -= sub_val
    if (u_n.lo < sub_val.lo) u_n.hi--;
    u_n.lo -= sub_val.lo;
    u_n.hi -= sub_val.hi;
    
    // --- Compute q0 (Low 32 bits) ---
    uint64_t q0 = 0;
    uint64_t q0_hat = 0;
    if (u_n.hi >= v_hi) {
        q0_hat = 0xFFFFFFFF;
    } else {
        q0_hat = ((u_n.hi << 32) | (u_n.lo >> 32)) / v_hi;
    }
    
    uint64_t r0_hat = ((u_n.hi << 32) | (u_n.lo >> 32)) - q0_hat * v_hi;
    
    // Refine
    while (q0_hat > 0xFFFFFFFF || (q0_hat * v_lo > ((r0_hat << 32) | (u_n.lo & 0xFFFFFFFF)))) {
        q0_hat--;
        r0_hat += v_hi;
        if (r0_hat > 0xFFFFFFFF && (r0_hat & 0xFFFFFFFF00000000ULL)) break; // Prevent infinite loop on overflow
    }
    q0 = q0_hat;
    
    return (q1 << 32) | q0;
}

// 128-bit addition (a + b)
static inline uint128_val add128(uint128_val a, uint128_val b) {
    uint128_val res;
    res.lo = a.lo + b.lo;
    res.hi = a.hi + b.hi + (res.lo < a.lo ? 1 : 0);
    return res;
}

// 128-bit subtraction (a - b)
static inline uint128_val sub128(uint128_val a, uint64_t b) {
    uint128_val res = a;
    if (res.lo < b) res.hi--;
    res.lo -= b;
    return res;
}

class RangeDecoder {
public:
    static const uint64_t TopValue = 1ULL << 56; // 1 << (32 + 24)
    static const uint64_t BottomValue = 1ULL << 24;

    inline RangeDecoder(const uint8_t* data, size_t dataSize, FrequencyModel& model)
        : _data(data), _dataSize(dataSize), _model(model), _byteIndex(0),
          _low(0), _range(0xFFFFFFFFFFFFFFFFULL), _code(0), _context(0) {
        
        // Initialize decoder state
        // The C# code reads 8 bytes.
        for (int i = 0; i < 8; ++i) {
            _code = (_code << 8) | readByte();
        }
    }
    
    inline int readBit() {
        return (int)readSymbol();
    }

    inline uint32_t readBits(int count) {
        uint32_t value = 0;
        for (int i = 0; i < count; ++i) {
            value = (value << 1) | (uint32_t)readBit();
        }
        return value;
    }

    inline uint32_t readSymbol() {
        uint32_t context = _context;
        uint32_t total = _model.getTotalFrequencies(context);

        // High precision calculation matching C# BigInteger implementation
        
        // Calculate scaledCode first using full precision
        // scaledCode = ((_code - _low + 1) * total - 1) / _range
        uint64_t diff64 = _code - _low + 1;
        uint128_val prod = mul64(diff64, (uint64_t)total);
        prod = sub128(prod, 1);
        uint64_t scaledCode64 = div128by64(prod, _range);
        uint32_t scaledCode = (uint32_t)scaledCode64;

        if (scaledCode >= total) scaledCode = total - 1;

        uint32_t symbolFreq;
        uint32_t symbol = _model.getSymbolFromCumulativeFrequency(scaledCode, context, symbolFreq);
        uint32_t cumulative = _model.getCumulativeFrequency(symbol, context);

        // Update range and low using optimized calculation
        // Optimization: _range = total * q + r
        // val * _range / total = val * q + (val * r) / total
        
        // Pre-calculate Quotient and Remainder of _range / total
        uint64_t rQuot = _range / total;
        uint64_t rRem = _range % total;

        // Calculate newLow
        uint128_val term1 = mul64x32(rQuot, cumulative);
        uint64_t term2_val = ((uint64_t)cumulative * rRem) / total;
        uint128_val newLow128 = add128(term1, {term2_val, 0});
        newLow128 = add128(newLow128, {_low, 0});

        // Calculate newRange
        uint128_val rangeTerm1 = mul64x32(rQuot, symbolFreq);
        uint64_t rangeTerm2_val = ((uint64_t)symbolFreq * rRem) / total;
        uint128_val newRange128 = add128(rangeTerm1, {rangeTerm2_val, 0});
        
        uint64_t newRange64 = newRange128.lo; 
        
        if (newRange64 == 0) newRange64 = 1;
        
        _low = newLow128.lo;
        _range = newRange64;

        _model.update(symbol, context);

        if (_model.getNumContexts() > 1) {
            _context = symbol;
        }

        normalize();
        return symbol;
    }
    
    inline bool isEndOfStream() const {
        return _byteIndex >= _dataSize && _range == 0;
    }
    
    inline void setContext(uint32_t context) {
        _context = context;
    }

private:
    inline void normalize() {
        while ((_low ^ (_low + _range)) < TopValue || _range < BottomValue) {
            _code = (_code << 8) | readByte();
            _low <<= 8;
            _range <<= 8;
        }
    }

    inline uint8_t readByte() {
        if (_byteIndex < _dataSize) {
            return _data[_byteIndex++];
        }
        return 0;
    }

    const uint8_t* _data;
    size_t _dataSize;
    size_t _byteIndex;
    FrequencyModel& _model;

    uint64_t _low;
    uint64_t _range;
    uint64_t _code;
    uint32_t _context;
};

#endif // RANGE_DECODER_H