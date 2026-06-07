#include "tmg1/rice_writer.h"

RiceBitWriter::RiceBitWriter(Tmg1Stream& out, bool msbFirst)
    : _out(out), _msbFirst(msbFirst), _bitBuffer(0), _bitCount(0), _bytesWritten(0)
{}

void RiceBitWriter::writeBit(int bit) {
    if (_msbFirst) {
        _bitBuffer = (uint8_t)((_bitBuffer << 1) | (bit & 1));
    } else {
        _bitBuffer |= (uint8_t)((bit & 1) << _bitCount);
    }
    _bitCount++;
    if (_bitCount == 8) {
        writeByte(_bitBuffer);
        _bitBuffer = 0;
        _bitCount  = 0;
    }
}

void RiceBitWriter::writeBits(uint32_t value, int count) {
    // MSBから順に書き込む
    for (int i = count - 1; i >= 0; --i) {
        writeBit((int)((value >> i) & 1));
    }
}

void RiceBitWriter::writeSymbol(uint32_t symbol, int parameter) {
    uint32_t q = symbol >> parameter;
    uint32_t r = symbol & ((1u << parameter) - 1);

    // 商をunary符号で書く (1×q 個の後に 0)
    for (uint32_t i = 0; i < q; ++i) writeBit(1);
    writeBit(0);

    // 剰余をバイナリで書く
    writeBits(r, parameter);
}

int RiceBitWriter::flush() {
    if (_bitCount > 0) {
        if (_msbFirst) {
            _bitBuffer <<= (8 - _bitCount);
        }
        if (writeByte(_bitBuffer) < 0) return -1;
        _bitBuffer = 0;
        _bitCount  = 0;
    }
    return 0;
}

int RiceBitWriter::writeByte(uint8_t byte) {
    if (_out.write(_out.ctx, &byte, 1) != 1) return -1;
    _bytesWritten++;
    return 0;
}
