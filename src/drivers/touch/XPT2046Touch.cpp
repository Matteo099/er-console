#include "XPT2046Touch.h"
#include "log/Logger.h"

XPT2046Touch::XPT2046Touch()
    : touch(TOUCH_CS_PIN, TOUCH_IRQ_PIN), touchSPI(HSPI) {}

void XPT2046Touch::init()
{
    Log::info(Log::DRIVER_TAG, "Initializing Touch XPT2046");
    touchSPI.begin(SPI1_SCLK_PIN, SPI1_MISO_PIN, SPI1_MOSI_PIN);
    Log::info(Log::DRIVER_TAG, "touchSPI.begin(SPI1_SCLK_PIN, SPI1_MISO_PIN, SPI1_MOSI_PIN)");
    touch.begin(touchSPI);       // Pass SPI instance here
    Log::info(Log::DRIVER_TAG, "touch.begin(touchSPI)");
    touch.setRotation(1);
    Log::info(Log::DRIVER_TAG, "touch.setRotation(1)");
    pinMode(TOUCH_IRQ_PIN, INPUT_PULLUP);
    Log::info(Log::DRIVER_TAG, "IRQ pin configured");
}

TouchPoint XPT2046Touch::read()
{
    int irqState = digitalRead(TOUCH_IRQ_PIN);
    TouchPoint p{0, 0, false};

    if (irqState == LOW || touch.touched()) {
        TS_Point raw = touch.getPoint();
        p.x = raw.x;
        p.y = raw.y;
        p.pressed = true;
    }

    return p;
}
