#pragma once
#include "DisplayDriver.h"
#include <TFT_eSPI.h>

class TFTDisplay : public DisplayDriver {
public:
    TFTDisplay();

    void init() override;
    void clear(uint16_t color) override;

    void drawText(int x, int y, const char* text, uint16_t color) override;

    void drawRect(int x, int y, int w, int h, uint16_t color) override;
    void fillRect(int x, int y, int w, int h, uint16_t color) override;

    int width() const override;
    int height() const override;

private:
    TFT_eSPI tft;
};
