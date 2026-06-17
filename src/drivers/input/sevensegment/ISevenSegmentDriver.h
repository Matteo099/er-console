#pragma once

#include <stdint.h>


class ISevenSegmentDriver
{
public:

    virtual ~ISevenSegmentDriver() = default;


    virtual bool init() = 0;


    virtual bool available() const = 0;


    virtual uint8_t digits() const = 0;


    virtual void clear() = 0;


    virtual void setDigit(
        uint8_t digit,
        char value,
        bool dot=false) = 0;


    virtual void setBrightness(
        uint8_t value) = 0;
};