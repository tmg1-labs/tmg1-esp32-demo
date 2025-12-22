#pragma once

#include <cstdint>
#include <cstddef>
#include <string.h> // for memcpy, memset

// Minimal Stream class definition to satisfy the compiler for native tests
class Stream {
public:
    virtual ~Stream() {}
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t readBytes(char *buffer, size_t length) = 0;
    // Add other virtual methods your code might need, e.g., write, flush
    virtual size_t write(uint8_t) { return 0; }
    virtual void flush() {}
};

// Other minimal definitions if needed by the code under test
void delay(uint32_t ms);
