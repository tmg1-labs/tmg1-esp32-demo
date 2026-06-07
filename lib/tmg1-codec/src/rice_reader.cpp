#include "tmg1/rice_reader.h"

RiceBitReader::RiceBitReader(const uint8_t* src, size_t srcSize, bool msbFirst)
    : _src(src), _srcSize(srcSize), _msbFirst(msbFirst), _byteIndex(0), _bitIndex(0)
{}

int RiceBitReader::readBit() {
    if (isEndOfStream()) return -1;

    uint8_t cur = _src[_byteIndex];
    int bit;
    if (_msbFirst) {
        bit = (cur >> (7 - _bitIndex)) & 1;
    } else {
        bit = (cur >> _bitIndex) & 1;
    }

    _bitIndex++;
    if (_bitIndex == 8) {
        _bitIndex = 0;
        _byteIndex++;
    }
    return bit;
}

uint32_t RiceBitReader::readBits(int count) {
    uint32_t value = 0;
    for (int i = 0; i < count; ++i) {
        int bit = readBit();
        if (bit == -1) return 0xFFFFFFFFu;
        // マルチビット値は書き込み順(MSBから)に読む
        value = (value << 1) | (uint32_t)bit;
    }
    return value;
}

uint32_t RiceBitReader::readSymbol(int parameter) {
    // 商: 1のrunをカウント
    uint32_t q = 0;
    while (readBit() == 1) q++;

    // 剰余: k ビットをバイナリで読む
    uint32_t r = readBits(parameter);
    if (r == 0xFFFFFFFFu) return 0xFFFFFFFFu;

    return (q << parameter) | r;
}

bool RiceBitReader::isEndOfStream() const {
    return _byteIndex >= _srcSize;
}
