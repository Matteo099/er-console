#include "TFTDisplay.h"
#include "log/Logger.h"

TFTDisplay::TFTDisplay() : tft() {}

void TFTDisplay::init()
{
    Log::info(Log::DISPLAY_TAG, "Initializing TFTDisplay");
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

void TFTDisplay::flush(
    int32_t x1, int32_t y1,
    int32_t x2, int32_t y2,
    const uint16_t* colorMap
)
{
    int32_t w = x2 - x1 + 1;
    int32_t h = y2 - y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(x1, y1, w, h);
    tft.pushColors((uint16_t*)colorMap, w * h, true);
    tft.endWrite();
}

int TFTDisplay::width()  { return tft.width(); }
int TFTDisplay::height() { return tft.height(); }
