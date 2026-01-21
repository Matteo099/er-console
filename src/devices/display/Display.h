#pragma once
#include <stdint.h>

class Display {
public:
    virtual ~Display() {}

    virtual void init() = 0;
    virtual void clear(uint16_t color) = 0;

    virtual void drawText(int x, int y, const char* text, uint16_t color) = 0;

    virtual void drawRect(int x, int y, int w, int h, uint16_t color) = 0;
    virtual void fillRect(int x, int y, int w, int h, uint16_t color) = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
};
