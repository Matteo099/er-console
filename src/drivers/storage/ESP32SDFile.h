#pragma once

#include <SD.h>
#include "IFile.h"
#include "drivers/bus/SPIBusLock.h"

class ESP32SDFile : public IFile
{
public:
    explicit ESP32SDFile(File f) : file(f) {}

    size_t length() const override
    {
        SPIBusLock::Guard guard;
        return file.size();
    }

    int read(uint8_t *buffer, size_t len) override
    {
        SPIBusLock::Guard guard;
        return file.read(buffer, len);
    }

    bool seek(size_t pos) override
    {
        SPIBusLock::Guard guard;
        return file.seek(pos);
    }

    bool available() override
    {
        SPIBusLock::Guard guard;
        return file.available();
    }

    void close() override
    {
        SPIBusLock::Guard guard;
        file.close();
    }

    size_t write(const uint8_t *buffer, size_t len) override
    {
        SPIBusLock::Guard guard;
        return file.write(buffer, len);
    }

    bool writeLine(const char *text, bool flush) override
    {
        SPIBusLock::Guard guard;

        size_t written = file.println(text);

        if (flush)
            file.flush();

        return written > 0;
    }

    void flush() override
    {
        SPIBusLock::Guard guard;
        file.flush();
    }

private:
    File file;
};
