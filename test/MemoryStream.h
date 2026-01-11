#pragma once
#include "Arduino.h"

class MemoryStream : public Stream {
 public:
  MemoryStream(const uint8_t* data, size_t size) : _data(data), _size(size), _position(0) {}

  virtual int available() { return _size - _position; }
  virtual int read() { return _position < _size ? _data[_position++] : -1; }
  virtual int peek() { return _position < _size ? _data[_position] : -1; }
  virtual void flush() {}
  virtual size_t write(uint8_t) { return 0; }  // Not implemented for reading

  // For Tmg1Decoder which uses readBytes
  virtual size_t readBytes(char* buffer, size_t length) {
    size_t count = 0;
    while (count < length && available()) {
      *buffer++ = read();
      count++;
    }
    return count;
  }

 private:
  const uint8_t* _data;
  size_t _size;
  size_t _position;
};
