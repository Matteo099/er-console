#include "SSD1306Driver.h"

#include "config/BoardConfig.h"
#include "log/Logger.h"

SSD1306Driver::SSD1306Driver(
    uint16_t width,
    uint16_t height,
    TwoWire &wire,
    uint8_t address) : displayWidth(width), displayHeight(height), displayAddress(address), wireBus(&wire), display(width, height, &wire, -1) {}

bool SSD1306Driver::init()
{
    wireBus->begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
    if (!display.begin(SSD1306_SWITCHCAPVCC, displayAddress))
    {
        Log::warn(Log::DRIVER_TAG, "SSD1306 not detected");
        initialized = false;
        return false;
    }
    display.clearDisplay();
    display.display();
    initialized = true;
    Log::info(Log::DRIVER_TAG, "SSD1306 initialized");
    return true;
}

bool SSD1306Driver::isAvailable() const { return initialized; }

void SSD1306Driver::clear()
{
    if (!initialized)
        return;
    display.clearDisplay();
}

void SSD1306Driver::print(const std::string &text, uint16_t x, uint16_t y)
{
    if (!initialized)
        return;
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(x, y);
    display.println(text.c_str());
}

void SSD1306Driver::drawPixel(uint16_t x, uint16_t y)
{
    if (!initialized)
        return;
    display.drawPixel(x, y, SSD1306_WHITE);
}

void SSD1306Driver::refresh()
{
    if (!initialized)
        return;
    display.display();
}

DisplaySize SSD1306Driver::size() const
{
    DisplaySize size;

    size.width = displayWidth;
    size.height = displayHeight;

    return size;
}