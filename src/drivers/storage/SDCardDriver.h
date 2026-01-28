#pragma once
#include <stdint.h>

class SDCardDriver
{
public:
    virtual ~SDCardDriver() = default;

    virtual bool init() = 0;
    virtual bool mounted() const = 0;
};
