#include "XPT2046Touch.h"
#include "log/Logger.h"

XPT2046Touch::XPT2046Touch()
    : touch(TOUCH_CS, TOUCH_IRQ), touchSPI(HSPI) {}

void XPT2046Touch::init()
{
    Log::info(Log::DRIVER_TAG, "Initializing Touch XPT2046");
    touchSPI.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI);
    Log::info(Log::DRIVER_TAG, "touchSPI.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI)");
    touch.begin(touchSPI);       // Pass SPI instance here
    Log::info(Log::DRIVER_TAG, "touch.begin(touchSPI)");
    touch.setRotation(1);
    Log::info(Log::DRIVER_TAG, "touch.setRotation(1)");
    pinMode(TOUCH_IRQ, INPUT_PULLUP);
    Log::info(Log::DRIVER_TAG, "IRQ pin configured");
}

TouchPoint XPT2046Touch::read()
{
    int irqState = digitalRead(TOUCH_IRQ);
    TouchPoint p{0, 0, false};

    if (irqState == LOW || touch.touched()) {
        TS_Point raw = touch.getPoint();
        p.x = raw.x;
        p.y = raw.y;
        p.pressed = true;
    }

    return p;
}
