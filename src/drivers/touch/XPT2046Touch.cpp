#include "XPT2046Touch.h"
#include "log/Logger.h"

XPT2046Touch::XPT2046Touch(uint8_t csPin)
    : ts(csPin) {}

void XPT2046Touch::init()
{
    Log::info(Log::DRIVER_TAG, "Initializing Touch XPT2046");
    ts.begin();
    ts.setRotation(1);
}

TouchPoint XPT2046Touch::read()
{
    TouchPoint p{0, 0, false};

    if (ts.touched()) {
        TS_Point raw = ts.getPoint();
        p.x = raw.x;
        p.y = raw.y;
        p.pressed = true;
    }

    return p;
}
