#pragma once

#include <cstddef>

class IFile {
public:
    virtual ~IFile() = default;

    virtual int read(uint8_t* buffer, size_t len) = 0;
    virtual bool seek(size_t pos) = 0;
    virtual bool available() = 0;
    virtual void close() = 0;
};
