#pragma once

#include <SD.h>
#include "IFile.h"
#include "SDLock.h"

class ESP32SDFile : public IFile {
public:
    explicit ESP32SDFile(File f) : file(f) {}

    int read(uint8_t* buffer, size_t len) override {
        SDLock::Guard guard;
        return file.read(buffer, len);
    }

    bool seek(size_t pos) override {
        SDLock::Guard guard;
        return file.seek(pos);
    }

    bool available() override {
        SDLock::Guard guard;
        return file.available();
    }

    void close() override {
        SDLock::Guard guard;
        file.close();
    }

private:
    File file;
};
