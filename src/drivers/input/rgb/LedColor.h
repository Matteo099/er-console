#pragma once

#include <stdint.h>

struct LedColor
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;


    bool operator==(const LedColor& other) const
    {
        return
            r == other.r &&
            g == other.g &&
            b == other.b;
    }
};