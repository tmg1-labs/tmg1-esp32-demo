#pragma once
#include <stddef.h>
#include <stdint.h>

// Riceビット読み取り: #include <Arduino.h> 依存なし
class RiceBitReader {
public:
    RiceBitReader(const uint8_t* src, size_t srcSize, bool msbFirst);

    int      readBit();
    uint32_t readBits(int count);
    // Rice符号を読む: q をunary、r をbinaryで解釈して (q << parameter | r) を返す
    uint32_t readSymbol(int parameter);
    bool     isEndOfStream() const;

private:
    const uint8_t* _src;
    size_t         _srcSize;
    bool           _msbFirst;
    size_t         _byteIndex;
    uint8_t        _bitIndex; // 0-7
};
