#pragma once

#include <SD.h>
#include <vector>
#include <string>
#include "core/SystemService.h"
#include "drivers/storage/SDCardDriver.h"
#include "drivers/storage/IFile.h"
#include "drivers/storage/StorageEntry.h"

class StorageSystem : public SystemService
{
public:
    StorageSystem();

    void init(void *params = nullptr) override;
    bool isReady() const;
    void update() override {};

    virtual IFile* open(const char* path);
    bool readTextFile(const char* path, std::string& out);
    bool writeTextFile(const char* path, const std::string& contents, bool overwrite = true);
    File openNative(const char* path, bool write = false);
    bool removePath(const char* path);
    bool movePath(const char* sourcePath, const char* destinationPath, bool overwrite = true);
    bool createDirectory(const char* path);
    bool exists(const char* path) const;
    bool listDirectory(const char* path, std::vector<StorageEntry>& out, bool recursive = false) const;
    uint64_t totalBytes() const;
    uint64_t usedBytes() const;
    uint64_t freeBytes() const;
    void listFiles() const;

private:
    SDCardDriver* driver;
    bool ready = false;
};
