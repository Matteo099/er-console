#pragma once
#include <stdint.h>

class DisplayDriver
{
public:
    virtual ~DisplayDriver() = default;

    virtual void init() = 0;

    // Called by LVGL flush callback
    virtual void flush(
        int x1, int y1,
        int x2, int y2,
        uint8_t *colorData) = 0;

    virtual int width() = 0;
    virtual int height() = 0;
};
