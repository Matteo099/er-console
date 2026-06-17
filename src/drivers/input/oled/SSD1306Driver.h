#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "IDisplay.h"

class SSD1306Driver : public IDisplay
{
public:
    SSD1306Driver(
        uint16_t width = 128,
        uint16_t height = 64,
        TwoWire& wire = Wire,
        uint8_t address = 0x3C);

    bool init() override;

    bool isAvailable() const override;

    void clear() override;

    void print(
        const std::string& text,
        uint16_t x = 0,
        uint16_t y = 0) override;

    void drawPixel(
        uint16_t x,
        uint16_t y) override;

    void refresh() override;

    DisplaySize size() const override;

private:
    uint16_t displayWidth;
    uint16_t displayHeight;

    uint8_t displayAddress;

    TwoWire* wireBus;

    bool initialized = false;

    Adafruit_SSD1306 display;
};