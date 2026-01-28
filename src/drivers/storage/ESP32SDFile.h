#pragma once

#include <SD.h>
#include "IFile.h"

class ESP32SDFile : public IFile {
public:
    explicit ESP32SDFile(File f) : file(f) {}

    int read(uint8_t* buffer, size_t len) override {
        return file.read(buffer, len);
    }

    bool seek(size_t pos) override {
        return file.seek(pos);
    }

    bool available() override {
        return file.available();
    }

    void close() override {
        file.close();
    }

private:
    File file;
};
