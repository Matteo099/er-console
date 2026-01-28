#pragma once

#include <stdint.h>
#include "drivers/storage/IFile.h"

class SDCardDriver
{
public:
    virtual ~SDCardDriver() = default;

    virtual bool init() = 0;
    virtual bool mounted() const = 0;
    virtual IFile* open(const char* path) = 0;
};
