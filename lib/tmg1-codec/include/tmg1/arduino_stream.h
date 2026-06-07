#pragma once
// Arduino向け: Stream/File を Tmg1Stream にラップするアダプタ
// このヘッダは Arduino 環境でのみ有効
#ifdef ARDUINO
#include <Arduino.h>
#include "io.h"

// Arduino Stream から読み込むコールバック
inline int tmg1_arduino_read(void* ctx, uint8_t* buf, size_t len) {
    Stream* s = static_cast<Stream*>(ctx);
    return (int)s->readBytes(reinterpret_cast<char*>(buf), len);
}

// Arduino Stream (読み取り専用) を Tmg1Stream にラップする
inline Tmg1Stream tmg1_stream_from_arduino(Stream& s) {
    Tmg1Stream st = {};
    st.ctx   = &s;
    st.read  = tmg1_arduino_read;
    st.write = nullptr;
    st.tell  = nullptr;
    st.seek  = nullptr;
    return st;
}

#endif // ARDUINO
