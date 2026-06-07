#pragma once
#include <stdint.h>
#include <stddef.h>
#include <algorithm>
#include <vector>

// 適応型頻度モデル: Rangeコーダのシンボル確率を管理する
// ESP32/デスクトップ共通 (std::vector はArduinoでも使用可能)
class FrequencyModel {
public:
    FrequencyModel(uint32_t maxSymbols, uint32_t numContexts = 1)
        : _maxSymbols(maxSymbols), _numContexts(numContexts)
    {
        _frequencies.resize(_numContexts * _maxSymbols);
        _totals.resize(_numContexts);
        resetAll();
    }

    // 累積頻度 [0, symbol) の合計を返す
    inline uint32_t getCumulativeFrequency(uint32_t symbol, uint32_t context) const {
        uint32_t cumulative = 0;
        size_t base = context * _maxSymbols;
        for (uint32_t i = 0; i < symbol; ++i) {
            cumulative += _frequencies[base + i];
        }
        return cumulative;
    }

    inline uint32_t getSymbolFrequency(uint32_t symbol, uint32_t context) const {
        if (symbol >= _maxSymbols) return 0;
        return _frequencies[context * _maxSymbols + symbol];
    }

    inline uint32_t getTotalFrequencies(uint32_t context) const {
        return _totals[context];
    }

    // scaledCode から対応するシンボルを線形探索で返す
    // symbolFrequency に該当シンボルの頻度を書き込む
    inline uint32_t getSymbolFromCumulativeFrequency(
            uint32_t scaledCode, uint32_t context,
            uint32_t& symbolFrequency) const
    {
        uint32_t cum = 0;
        size_t base = context * _maxSymbols;
        for (uint32_t i = 0; i < _maxSymbols; ++i) {
            symbolFrequency = _frequencies[base + i];
            cum += symbolFrequency;
            if (scaledCode < cum) return i;
        }
        symbolFrequency = 0;
        return 0; // 到達不能 (total > 0 が保証されている前提)
    }

    inline void update(uint32_t symbol, uint32_t context) {
        if (symbol >= _maxSymbols) return;
        _frequencies[context * _maxSymbols + symbol]++;
        _totals[context]++;
        if (_totals[context] > (1u << 20)) rescale(context);
    }

    // バイナリモデル専用: freq(0) を直接取得する
    inline uint32_t getZeroFrequency(uint32_t context) const {
        return _frequencies[context * _maxSymbols];
    }

    // バイナリモデル専用の高速update
    inline void updateBinary(uint32_t symbol, uint32_t context) {
        _frequencies[context * _maxSymbols + symbol]++;
        _totals[context]++;
        if (_totals[context] > (1u << 20)) rescale(context);
    }

    inline void reset() { resetAll(); }

    inline uint32_t getNumContexts() const { return _numContexts; }

private:
    inline void resetAll() {
        for (uint32_t ctx = 0; ctx < _numContexts; ++ctx) {
            _totals[ctx] = 0;
            for (uint32_t s = 0; s < _maxSymbols; ++s) {
                _frequencies[ctx * _maxSymbols + s] = 1;
                _totals[ctx]++;
            }
        }
    }

    // 頻度が大きくなりすぎた場合に各頻度を半分にする
    inline void rescale(uint32_t context) {
        size_t base = context * _maxSymbols;
        _totals[context] = 0;
        for (uint32_t i = 0; i < _maxSymbols; ++i) {
            uint32_t newFreq = std::max(1u, _frequencies[base + i] / 2);
            _frequencies[base + i] = newFreq;
            _totals[context] += newFreq;
        }
    }

    uint32_t _maxSymbols;
    uint32_t _numContexts;
    std::vector<uint32_t> _frequencies; // [context * maxSymbols + symbol]
    std::vector<uint32_t> _totals;
};
