#pragma once
#include <stddef.h>
#include <stdint.h>
#include "freq_model.h"
#include "io.h"

// Rangeエンコーダ (64bit, __uint128_t不要)
// 正規化: XOR条件 + BottomValue でバイトが確定次第出力 (C#参照と同一ロジック)
// 演算: divide-first (step=_range/total先算) で積がuint64に収まる
class RangeEncoder {
public:
    static const uint64_t TopValue    = 1ULL << 56;
    static const uint64_t BottomValue = 1ULL << 24;

    RangeEncoder(FrequencyModel& model, Tmg1Stream& out);

    void writeBit(int bit);
    void writeBits(uint32_t value, int count);
    void writeSymbol(uint32_t symbol);

    int  flush();

    size_t bytesWritten() const { return _bytesWritten; }
    void   setContext(uint32_t context) { _context = context; }

private:
    void normalize();
    int  writeByte(uint8_t byte);

    FrequencyModel& _model;
    Tmg1Stream&     _out;
    uint64_t        _low;
    uint64_t        _range;
    uint32_t        _context;
    size_t          _bytesWritten;
};
