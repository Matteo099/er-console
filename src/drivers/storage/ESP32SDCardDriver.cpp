#include "ESP32SDCardDriver.h"

ESP32SDCardDriver::ESP32SDCardDriver(uint8_t csPin)
    : cs(csPin) {}

bool ESP32SDCardDriver::init()
{
    isMounted = SD.begin(cs);
    return isMounted;
}

bool ESP32SDCardDriver::mounted() const
{
    return isMounted;
}
