#pragma once

#include <stdint.h>
#include <vector>
#include "IFile.h"
#include "StorageEntry.h"

enum class FileMode
{
    Read,
    Write,
    Append
};

class SDCardDriver
{
public:
    virtual ~SDCardDriver() = default;

    virtual bool init() = 0;
    virtual bool mounted() const = 0;
    virtual IFile *open(const char *path, FileMode mode = FileMode::Read) = 0;
    virtual bool exists(const char *path) const = 0;
    virtual bool remove(const char *path) = 0;
    virtual bool rename(const char *src, const char *dst) = 0;
    virtual bool mkdir(const char *path) = 0;
    virtual bool listDirectory(const char *path, std::vector<StorageEntry> &out) const = 0;
    virtual uint64_t totalBytes() const = 0;
    virtual uint64_t usedBytes() const = 0;
    virtual bool ensureParents(const char *path) = 0;
};