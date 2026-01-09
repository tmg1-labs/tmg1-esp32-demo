#include "Tmg1Decoder.h"
#include "RangeDecoder.h"
#include "RiceBitReader.h"
#include <new>
#include <stdio.h> // For printf debugging

Tmg1Decoder::Tmg1Decoder() : _rangeModel(2, 2) {
    memset(&_fileHeader, 0, sizeof(_fileHeader));
    _previousFrame = nullptr;
    _tempFrame = nullptr;
    _stream = nullptr;
    _frameBufferSize = 0;
    _tempFrameSize = 0;
    _lastPtsDelta = 0;
}

Tmg1Decoder::~Tmg1Decoder() {
    if (_previousFrame) {
        delete[] _previousFrame;
    }
    if (_tempFrame) {
        delete[] _tempFrame;
    }
}

bool Tmg1Decoder::begin(Stream& stream) {
    _stream = &stream;
    if (readFileHeader() != Tmg1DecoderError::None) {
        return false;
    }

    _frameBufferSize = getWidth() * getHeight(); // Assuming 1 bit per pixel for now
    if (_frameBufferSize % 8 != 0) {
        _frameBufferSize = (_frameBufferSize / 8) + 1;
    } else {
        _frameBufferSize /= 8;
    }
    
    // Allocate buffers if not already allocated or if size changed (though video size shouldn't change mid-stream)
    if (!_previousFrame) {
        _previousFrame = new (std::nothrow) uint8_t[_frameBufferSize];
    }
    if (!_previousFrame) {
        return false;
    }
    
    if (!_tempFrame) {
        _tempFrame = new (std::nothrow) uint8_t[_frameBufferSize];
        _tempFrameSize = _frameBufferSize;
    }
    if (!_tempFrame) {
        return false;
    }

    // Reset range model just in case, though it's reset per frame usage usually
    _rangeModel.reset();
    
    return true;
}

bool Tmg1Decoder::decodeFrame(uint8_t* buffer, size_t bufferSize) {
    if (bufferSize < _frameBufferSize) {
        return false; // Output buffer is too small
    }

    Tmg1FrameHeader frameHeader;
    Tmg1DecoderError err = readFrameHeader(frameHeader);
    if (err != Tmg1DecoderError::None) {
        return false;
    }
    _lastPtsDelta = frameHeader.ptsDelta;

    // Handle empty payload (e.g., unchanged P-frame)
    if (frameHeader.payloadSize == 0) {
        if (frameHeader.frameType == 1) { // P-Frame
            // If the output buffer is not the same as the previous frame buffer, copy the content
            if (buffer != _previousFrame) {
                memcpy(buffer, _previousFrame, _frameBufferSize);
            }
            return true;
        } else { // I-Frame
            // Empty I-Frame, fill with zeros (black)
            memset(buffer, 0, _frameBufferSize);
            memcpy(_previousFrame, buffer, _frameBufferSize);
            return true;
        }
    }

    // Read payload from stream into reusable buffer
    if (_payloadBuffer.size() < frameHeader.payloadSize) {
        _payloadBuffer.resize(frameHeader.payloadSize);
    }
    
    if (_stream->readBytes(reinterpret_cast<char*>(_payloadBuffer.data()), frameHeader.payloadSize) != frameHeader.payloadSize) {
        return false;
    }

    bool success = false;
    if (frameHeader.frameType == 1) { // P-Frame
        // For P-Frames, we decompress the delta into a temporary buffer
        // Use pre-allocated _tempFrame
        if (!_tempFrame || _tempFrameSize < _frameBufferSize) {
             // Should have been allocated in begin, but safety check
             return false;
        }

        if (decompressPayload(_payloadBuffer.data(), frameHeader.payloadSize, _tempFrame, _frameBufferSize, frameHeader.frameFlags, frameHeader.frameType)) {
            applyInversePrediction(_tempFrame, _frameBufferSize, frameHeader.predictionMethod);
            // XOR with the previous frame to reconstruct the current frame
            for (size_t i = 0; i < _frameBufferSize; ++i) {
                buffer[i] = _previousFrame[i] ^ _tempFrame[i];
            }
            success = true;
        }
    } else { // I-Frame
        // For I-Frames, we decompress directly into the output buffer
        if (decompressPayload(_payloadBuffer.data(), frameHeader.payloadSize, buffer, bufferSize, frameHeader.frameFlags, frameHeader.frameType)) {
            applyInversePrediction(buffer, bufferSize, frameHeader.predictionMethod);
            success = true;
        }
    }

    if (success) {
        // Save the current frame for the next P-Frame
        memcpy(_previousFrame, buffer, _frameBufferSize);

        // Convert from MSB-First (TMG1 Standard) to LSB-First (U8g2 XBM format)
        // Only necessary if the display library expects LSB-First.
        // U8g2 drawXBMP expects LSB-First (Bit 0 is left-most pixel).
        for (size_t i = 0; i < bufferSize; ++i) {
            uint8_t b = buffer[i];
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            buffer[i] = b;
        }
    }

    return success;
}

uint16_t Tmg1Decoder::getWidth() const {
    return _fileHeader.width;
}

uint16_t Tmg1Decoder::getHeight() const {
    return _fileHeader.height;
}

uint16_t Tmg1Decoder::getTimebaseNum() const {
    return _fileHeader.timebaseNum;
}

uint16_t Tmg1Decoder::getTimebaseDen() const {
    return _fileHeader.timebaseDen;
}

uint32_t Tmg1Decoder::getLastPtsDelta() const {
    return _lastPtsDelta;
}

Tmg1DecoderError Tmg1Decoder::readFileHeader() {
    size_t readSize = _stream->readBytes(reinterpret_cast<char*>(&_fileHeader), sizeof(_fileHeader));
    if (readSize != sizeof(_fileHeader)) {
        return Tmg1DecoderError::ReadError;
    }

    // シグネチャ 'TMG1' はリトルエンディアンでは 0x31474D54
    if (_fileHeader.signature != 0x31474D54) {
        return Tmg1DecoderError::InvalidSignature;
    }

    if (_fileHeader.version != 1) {
        return Tmg1DecoderError::InvalidVersion;
    }

    return Tmg1DecoderError::None;
}

Tmg1DecoderError Tmg1Decoder::readFrameHeader(Tmg1FrameHeader& header) {
    if (_stream->readBytes(reinterpret_cast<char*>(&header.frameType), 1) != 1) {
        return Tmg1DecoderError::ReadError;
    }

    Tmg1DecoderError err;
    err = readUleb128(header.ptsDelta);
    if (err != Tmg1DecoderError::None) {
        return err;
    }

    err = readUleb128(header.payloadSize);
    if (err != Tmg1DecoderError::None) {
        return err;
    }

    if (_stream->readBytes(reinterpret_cast<char*>(&header.frameFlags), 1) != 1) {
        return Tmg1DecoderError::ReadError;
    }

    if (_stream->readBytes(reinterpret_cast<char*>(&header.predictionMethod), 1) != 1) {
        return Tmg1DecoderError::ReadError;
    }

    return Tmg1DecoderError::None;
}

Tmg1DecoderError Tmg1Decoder::readUleb128(uint32_t& value) {
    value = 0;
    uint8_t shift = 0;
    uint8_t byte;
    while (shift < 35) { // 5 bytes * 7 bits = 35 bits
        if (_stream->readBytes(reinterpret_cast<char*>(&byte), 1) != 1) {
            return Tmg1DecoderError::ReadError;
        }
        value |= (uint32_t)(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) {
            return Tmg1DecoderError::None;
        }
        shift += 7;
    }
    // ULEB128 for uint32_t should not exceed 5 bytes.
    // This indicates malformed data.
    return Tmg1DecoderError::ReadError;
}

bool Tmg1Decoder::decompressPayload(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags, uint8_t frameType) {
    bool isRangeCoder = (_fileHeader.flags & 0x04) != 0;

    if (isRangeCoder) {
        return decompressPayloadRange(src, srcSize, dest, destSize, frameFlags, frameType);
    } else {
        return decompressPayloadRice(src, srcSize, dest, destSize, frameFlags, frameType);
    }
}

bool Tmg1Decoder::decompressPayloadRange(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags, uint8_t frameType) {
    // Reuse existing model
    _rangeModel.reset(); 
    RangeDecoder reader(src, srcSize, _rangeModel);
    
    uint16_t width = getWidth();
    uint16_t height = getHeight();
    size_t bytesPerLine = (width + 7) / 8;
    bool msbFirst = (_fileHeader.flags & 0x01) != 0;

    memset(dest, 0, destSize);

    for (uint16_t y = 0; y < height; ++y) {
        if (reader.isEndOfStream()) break;
        
        int lineType = reader.readBit();
        if (lineType == 0) {
            continue;
        }

        for (uint16_t bitsDecoded = 0; bitsDecoded < width; ++bitsDecoded) {
            // Check for end of stream within line just in case
            if (reader.isEndOfStream()) break;
            
            int bit = reader.readBit();
            
            if (bit == 1) {
                size_t byteIndex = y * bytesPerLine + (bitsDecoded / 8);
                uint8_t bitInByte;
                if (msbFirst) {
                    bitInByte = 7 - (bitsDecoded % 8);
                } else {
                    bitInByte = bitsDecoded % 8;
                }
                dest[byteIndex] |= (1 << bitInByte);
            }
        }
    }
    return true;
}

bool Tmg1Decoder::decompressPayloadRice(const uint8_t* src, size_t srcSize, uint8_t* dest, size_t destSize, uint8_t frameFlags, uint8_t frameType) {
    // Rice decompression logic (unchanged)
    bool msbFirst = (_fileHeader.flags & 0x01) != 0;
    RiceBitReader reader(src, srcSize, msbFirst);

    bool hasStartBit = (frameFlags & 0x01) != 0;
    bool perLineK = (frameFlags & 0x02) != 0;
    bool perFrameK = (frameFlags & 0x04) != 0;

    int frameK = 1; // Default K for fixed mode
    if (perFrameK) {
        uint32_t k_val = reader.readBits(3);
        if (k_val == 0xFFFFFFFF) {
            printf("ERROR: Failed to read per-frame K.\n");
            return false;
        }
        frameK = k_val;
    }

    uint16_t width = getWidth();
    uint16_t height = getHeight();
    size_t bytesPerLine = (width + 7) / 8;

    memset(dest, 0, destSize);

    for (uint16_t y = 0; y < height; ++y) {
        int lineType = reader.readBit();
        if (lineType == -1) {
            printf("ERROR: Unexpected end of stream when reading lineType at y=%d.\n", y);
            return false;
        }

        if (lineType == 0) {
            continue;
        }

        int currentBit = 0;
        if (hasStartBit) {
            int bit = reader.readBit();
            if (bit == -1) {
                printf("ERROR: Unexpected end of stream when reading start bit at y=%d.\n", y);
                return false;
            }
            currentBit = bit;
        }

        int lineK = frameK;
        if (perLineK) {
            uint32_t k_val = reader.readBits(3);
            if (k_val == 0xFFFFFFFF) {
                printf("ERROR: Failed to read per-line K at y=%d.\n", y);
                return false;
            }
            lineK = k_val;
        }

        uint16_t bitsWrittenInLine = 0;
        while (bitsWrittenInLine < width) {
            if (reader.isEndOfStream()) {
                if(currentBit == 1) {
                    printf("ERROR: Stream ended unexpectedly on a white run at y=%d, bit %d.\n", y, bitsWrittenInLine);
                    return false;
                }
                break;
            }
            
            uint32_t runLength = reader.readSymbol(lineK);
            if (runLength == 0xFFFFFFFF) {
                printf("ERROR: Failed to read symbol at y=%d, bit %d.\n", y, bitsWrittenInLine);
                return false;
            }
            
            if (runLength == 0) {
                 currentBit = 1 - currentBit;
                 continue; 
            }

            if (currentBit == 1) {
                for (uint32_t i = 0; i < runLength && bitsWrittenInLine < width; ++i) {
                    size_t byte_idx = y * bytesPerLine + (bitsWrittenInLine / 8);
                    uint8_t bit_idx = bitsWrittenInLine % 8;
                    dest[byte_idx] |= (1 << bit_idx);
                    bitsWrittenInLine++;
                }
            } else {
                bitsWrittenInLine += runLength;
            }
            
            if (bitsWrittenInLine > width) {
                bitsWrittenInLine = width;
            }

            currentBit = 1 - currentBit; 
        }
        if (!hasStartBit) {
            currentBit = 0;
        }
    }
    return true;
}

void Tmg1Decoder::applyInversePrediction(uint8_t* buffer, size_t bufferSize, uint8_t predictionMethod) {
    if (predictionMethod == 0) { // None
        return;
    }

    uint16_t width = getWidth();
    uint16_t height = getHeight();
    size_t bytesPerLine = (width + 7) / 8;

    if (predictionMethod == 1) { // Left
        for (uint16_t y = 0; y < height; ++y) {
            uint8_t* line = buffer + y * bytesPerLine;
            for (size_t i = 1; i < bytesPerLine; ++i) {
                line[i] ^= line[i - 1];
            }
        }
    } else if (predictionMethod == 2) { // Up
        for (uint16_t y = 1; y < height; ++y) {
            uint8_t* currentLine = buffer + y * bytesPerLine;
            uint8_t* upperLine = buffer + (y - 1) * bytesPerLine;
            for (size_t i = 0; i < bytesPerLine; ++i) {
                currentLine[i] ^= upperLine[i];
            }
        }
    }
}