#pragma once
#include "DisplayDriver.h"
#include <TFT_eSPI.h>

class TFTDisplay : public DisplayDriver
{
public:
    TFTDisplay();

    void init() override;

    void flush(
        int32_t x1, int32_t y1,
        int32_t x2, int32_t y2,
        const uint16_t *colorMap) override;

    virtual int width() = 0;
    virtual int height() = 0;

private:
    TFT_eSPI tft;
};
