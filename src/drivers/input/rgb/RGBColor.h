#pragma once

#include <stdint.h>

struct RGBColor
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;


    bool operator==(const RGBColor& other) const
    {
        return
            r == other.r &&
            g == other.g &&
            b == other.b;
    }
};