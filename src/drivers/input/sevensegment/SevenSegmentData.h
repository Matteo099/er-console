#pragma once

#include <stdint.h>
#include <string>


struct SevenSegmentValue
{
    std::string text;

    bool decimal = false;

    uint8_t brightness = 8;
};