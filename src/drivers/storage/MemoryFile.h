#pragma once

#include "IFile.h"
#include <cstring>

class MemoryFile : public IFile
{
public:
    MemoryFile() = default;

    void assign(uint8_t *ptr, size_t len)
    {
        data = ptr;
        size = len;
        pos = 0;
    }

    void reset()
    {
        pos = 0;
    }

    size_t position() const
    {
        return pos;
    }

    size_t length() const override
    {
        return size;
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

    bool writeLine(const char *text, bool flush) override
    {
        (void)text;
        (void)flush;
        return 0; // read-only
    }

    void flush() override
    {
        // nothing to do
    }

private:
    uint8_t *data = nullptr;
    size_t size = 0;
    size_t pos = 0;
};
