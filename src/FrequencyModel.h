#ifndef FREQUENCY_MODEL_H
#define FREQUENCY_MODEL_H

#include <stdint.h>
#include <vector>

class FrequencyModel {
public:
    FrequencyModel(uint32_t maxSymbols, uint32_t numContexts = 1);

    uint32_t getCumulativeFrequency(uint32_t symbol, uint32_t context) const;
    uint32_t getSymbolFrequency(uint32_t symbol, uint32_t context) const;
    uint32_t getTotalFrequencies(uint32_t context) const;
    uint32_t getSymbolFromCumulativeFrequency(uint32_t cumulativeFrequency, uint32_t context, uint32_t& symbolFrequency) const;
    void update(uint32_t symbol, uint32_t context);

    uint32_t getNumContexts() const { return _numContexts; }

private:
    void rescale(uint32_t context);

    uint32_t _maxSymbols;
    uint32_t _numContexts;
    std::vector<uint32_t> _frequencies; // Flattened 2D array: [context * maxSymbols + symbol]
    std::vector<uint32_t> _totals;
};

#endif // FREQUENCY_MODEL_H
