#ifndef FREQUENCY_MODEL_H
#define FREQUENCY_MODEL_H

#include <stdint.h>

#include <algorithm>  // for std::max
#include <vector>

class FrequencyModel {
 public:
  inline FrequencyModel(uint32_t maxSymbols, uint32_t numContexts = 1)
      : _maxSymbols(maxSymbols), _numContexts(numContexts) {
    _frequencies.resize(_numContexts * _maxSymbols);
    _totals.resize(_numContexts);

    for (uint32_t i = 0; i < _numContexts; ++i) {
      _totals[i] = 0;
      for (uint32_t j = 0; j < _maxSymbols; ++j) {
        _frequencies[i * _maxSymbols + j] = 1;
        _totals[i]++;
      }
    }
  }

  inline uint32_t getCumulativeFrequency(uint32_t symbol, uint32_t context) const {
    uint32_t cumulative = 0;
    size_t baseIndex = context * _maxSymbols;
    for (uint32_t i = 0; i < symbol; ++i) {
      cumulative += _frequencies[baseIndex + i];
    }
    return cumulative;
  }

  inline uint32_t getSymbolFrequency(uint32_t symbol, uint32_t context) const {
    if (symbol >= _maxSymbols) return 0;
    return _frequencies[context * _maxSymbols + symbol];
  }

  inline uint32_t getTotalFrequencies(uint32_t context) const { return _totals[context]; }

  inline uint32_t getSymbolFromCumulativeFrequency(uint32_t cumulativeFrequency, uint32_t context,
                                                   uint32_t& symbolFrequency) const {
    uint32_t currentCumulative = 0;
    size_t baseIndex = context * _maxSymbols;

    for (uint32_t i = 0; i < _maxSymbols; ++i) {
      symbolFrequency = _frequencies[baseIndex + i];
      currentCumulative += symbolFrequency;
      if (cumulativeFrequency < currentCumulative) {
        return i;
      }
    }
    symbolFrequency = 0;
    return 0;  // Should not happen
  }

  inline void update(uint32_t symbol, uint32_t context) {
    if (symbol >= _maxSymbols) return;

    _frequencies[context * _maxSymbols + symbol]++;
    _totals[context]++;

    if (_totals[context] > (1 << 20)) {
      rescale(context);
    }
  }

  // Optimized for Binary (0/1) models
  inline uint32_t getZeroFrequency(uint32_t context) const {
    // Assume maxSymbols is 2. Frequency of 0 is at index 0.
    return _frequencies[context * _maxSymbols];
  }

  inline void updateBinary(uint32_t symbol, uint32_t context) {
    // Optimized update for binary models (no loop for total check if we trust caller, but let's keep rescale logic)
    size_t idx = context * _maxSymbols + symbol;
    _frequencies[idx]++;
    _totals[context]++;
    if (_totals[context] > (1 << 20)) {
      rescale(context);
    }
  }

  inline void reset() {
    for (uint32_t i = 0; i < _numContexts; ++i) {
      _totals[i] = 0;
      for (uint32_t j = 0; j < _maxSymbols; ++j) {
        _frequencies[i * _maxSymbols + j] = 1;
        _totals[i]++;
      }
    }
  }

  inline uint32_t getNumContexts() const { return _numContexts; }

 private:
  inline void rescale(uint32_t context) {
    size_t baseIndex = context * _maxSymbols;
    _totals[context] = 0;

    for (uint32_t i = 0; i < _maxSymbols; ++i) {
      uint32_t newFreq = std::max((uint32_t)1, _frequencies[baseIndex + i] / 2);
      _frequencies[baseIndex + i] = newFreq;
      _totals[context] += newFreq;
    }
  }

  uint32_t _maxSymbols;
  uint32_t _numContexts;
  std::vector<uint32_t> _frequencies;  // Flattened 2D array: [context * maxSymbols + symbol]
  std::vector<uint32_t> _totals;
};

#endif  // FREQUENCY_MODEL_H