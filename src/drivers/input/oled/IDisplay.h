#pragma once

#include <string>

#include "DisplayTypes.h"

class IDisplay
{
public:
    virtual ~IDisplay() = default;

    virtual bool init() = 0;

    virtual bool isAvailable() const = 0;

    virtual void clear() = 0;

    virtual void print(
        const std::string& text,
        uint16_t x = 0,
        uint16_t y = 0) = 0;

    virtual void drawPixel(
        uint16_t x,
        uint16_t y) = 0;

    virtual void refresh() = 0;

    virtual DisplaySize size() const = 0;
};