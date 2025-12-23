#include "FrequencyModel.h"
#include <algorithm> // for std::max

FrequencyModel::FrequencyModel(uint32_t maxSymbols, uint32_t numContexts)
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

uint32_t FrequencyModel::getCumulativeFrequency(uint32_t symbol, uint32_t context) const {
    uint32_t cumulative = 0;
    size_t baseIndex = context * _maxSymbols;
    for (uint32_t i = 0; i < symbol; ++i) {
        cumulative += _frequencies[baseIndex + i];
    }
    return cumulative;
}

uint32_t FrequencyModel::getSymbolFrequency(uint32_t symbol, uint32_t context) const {
    if (symbol >= _maxSymbols) return 0;
    return _frequencies[context * _maxSymbols + symbol];
}

uint32_t FrequencyModel::getTotalFrequencies(uint32_t context) const {
    return _totals[context];
}

uint32_t FrequencyModel::getSymbolFromCumulativeFrequency(uint32_t cumulativeFrequency, uint32_t context, uint32_t& symbolFrequency) const {
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
    return 0; // Should not happen
}

void FrequencyModel::update(uint32_t symbol, uint32_t context) {
    if (symbol >= _maxSymbols) return;

    _frequencies[context * _maxSymbols + symbol]++;
    _totals[context]++;

    if (_totals[context] > (1 << 20)) {
        rescale(context);
    }
}

void FrequencyModel::rescale(uint32_t context) {
    size_t baseIndex = context * _maxSymbols;
    _totals[context] = 0;
    
    for (uint32_t i = 0; i < _maxSymbols; ++i) {
        uint32_t newFreq = std::max((uint32_t)1, _frequencies[baseIndex + i] / 2);
        _frequencies[baseIndex + i] = newFreq;
        _totals[context] += newFreq;
    }
}
