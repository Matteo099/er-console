#pragma once

#include <string>
#include <cstdint>

struct StorageEntry
{
    std::string path;
    std::string name;
    bool isDirectory = false;
    uint64_t size = 0;
};