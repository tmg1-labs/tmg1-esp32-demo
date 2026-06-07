#pragma once
#include <stddef.h>
#include <stdint.h>
#include "io.h"

// Riceビット書き込み: Tmg1Stream へコールバック経由で出力する
class RiceBitWriter {
public:
    RiceBitWriter(Tmg1Stream& out, bool msbFirst);

    void writeBit(int bit);
    void writeBits(uint32_t value, int count);
    // Rice符号を書く: q をunary(1×q + 0)、r をk bitバイナリで出力
    void writeSymbol(uint32_t symbol, int parameter);
    // バッファをフラッシュ (MSBファーストの場合は末尾にパディング)
    int  flush();

    size_t bytesWritten() const { return _bytesWritten; }

private:
    int writeByte(uint8_t byte);

    Tmg1Stream& _out;
    bool        _msbFirst;
    uint8_t     _bitBuffer;
    int         _bitCount;
    size_t      _bytesWritten;
};
