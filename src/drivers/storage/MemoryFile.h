#pragma once

#include "IFile.h"
#include <cstring>

class MemoryFile : public IFile
{
public:
    MemoryFile(uint8_t *data, size_t size)
        : data(data),
          size(size),
          pos(0)
    {
    }

    int read(uint8_t *buffer, size_t len) override
    {
        if (pos >= size)
            return 0;

        size_t remaining = size - pos;

        size_t toRead =
            len < remaining ? len : remaining;

        memcpy(
            buffer,
            data + pos,
            toRead);

        pos += toRead;

        return toRead;
    }

    bool seek(size_t p) override
    {
        if (p > size)
            return false;

        pos = p;
        return true;
    }

    bool available() override
    {
        return pos < size;
    }

    void close() override
    {
        // nothing
        // Memory owned by cache
    }

    size_t write(const uint8_t *buffer, size_t len) override
    {
        (void)buffer;
        (void)len;

        return 0; // read-only
    }

    void flush() override
    {
        // nothing to do
    }

private:
    uint8_t *data;
    size_t size;
    size_t pos;
};