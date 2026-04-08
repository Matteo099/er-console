#pragma once
#include "DisplayDriver.h"
#include "TouchController.h"
#include <LovyanGFX.hpp>
#include "LGFX.h"

class TFTDisplay : public DisplayDriver, public TouchController
{
public:
    TFTDisplay();

    void init() override;

    void flush(
        int32_t x1, int32_t y1,
        int32_t x2, int32_t y2,
        uint8_t *colorMap) override;

    int width() override;
    int height() override;

    TouchPoint read() override;

private:
    LGFX lgfx;
};

