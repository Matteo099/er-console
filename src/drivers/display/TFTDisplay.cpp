#include "TFTDisplay.h"

TFTDisplay::TFTDisplay() : tft() {}

void TFTDisplay::init() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
}

void TFTDisplay::clear(uint16_t color) {
    tft.fillScreen(color);
}

void TFTDisplay::drawText(int x, int y, const char* text, uint16_t color) {
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(text, x, y);
}

void TFTDisplay::drawRect(int x, int y, int w, int h, uint16_t color) {
    tft.drawRect(x, y, w, h, color);
}

void TFTDisplay::fillRect(int x, int y, int w, int h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

int TFTDisplay::width() const {
    return tft.width();
}

int TFTDisplay::height() const {
    return tft.height();
}
