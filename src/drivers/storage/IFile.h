#pragma once

#include <cstddef>
#include <cstdint>

class IFile {
  public:
    virtual ~IFile() = default;

    virtual bool isMemoryBacked() const {
        return false;
    }

    virtual int read(uint8_t* buffer, size_t len) = 0;
    virtual size_t write(const uint8_t* buffer, size_t len) = 0;
    virtual bool writeLine(const char* text, bool flush = false) = 0;

    virtual bool seek(size_t pos) = 0;
    virtual bool available() = 0;
    virtual size_t length() const = 0;

    virtual void flush() = 0;
    virtual void close() = 0;
};
