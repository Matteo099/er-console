#pragma once

#include <SD.h>
#include "IFile.h"
#include "drivers/bus/SPIBusLock.h"

class ESP32SDFile : public IFile {
public:
    explicit ESP32SDFile(File f) : file(f) {}

    int read(uint8_t* buffer, size_t len) override {
        SPIBusLock::Guard guard;
        return file.read(buffer, len);
    }

    bool seek(size_t pos) override {
        SPIBusLock::Guard guard;
        return file.seek(pos);
    }

    bool available() override {
        SPIBusLock::Guard guard;
        return file.available();
    }

    void close() override {
        SPIBusLock::Guard guard;
        file.close();
    }

private:
    File file;
};
