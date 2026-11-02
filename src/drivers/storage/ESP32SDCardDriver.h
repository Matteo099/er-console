#pragma once

#include <SD.h>
#include "SDCardDriver.h"
#include "drivers/storage/IFile.h"

class ESP32SDCardDriver : public SDCardDriver
{
public:
    ESP32SDCardDriver(SPIClass &spi, uint8_t csPin);

    bool init() override;
    bool mounted() const override;
    IFile *open(const char *path, FileMode mode = FileMode::Read) override;
    File openNative(const char *path, bool write = false);
    bool exists(const char *path) const override;
    bool remove(const char *path) override;
    bool rename(const char *src, const char *dst) override;
    bool mkdir(const char *path) override;
    bool listDirectory(const char *path, std::vector<StorageEntry> &out) const override;
    uint64_t totalBytes() const override;
    uint64_t capacityBytes() const override;
    uint64_t usedBytes() const override;
    bool ensureParents(const char *path) override;

private:
    SPIClass &spi;
    uint8_t cs;
    bool isMounted = false;

    std::string parentPath(const char *path);
};
