#include "ESP32SDCardDriver.h"
#include "log/Logger.h"
#include "drivers/bus/SPIBusLock.h"
#include "ESP32SDFile.h"

ESP32SDCardDriver::ESP32SDCardDriver(SPIClass &spi, uint8_t csPin)
    : spi(spi), cs(csPin) {}

bool ESP32SDCardDriver::init()
{
    SPIBusLock::Guard guard;

    Log::info(Log::DRIVER_TAG, "Initializing SD Card (CS=%d)", cs);
    isMounted = SD.begin(cs, spi);
    return isMounted;
}

bool ESP32SDCardDriver::mounted() const
{
    return isMounted;
}

IFile* ESP32SDCardDriver::open(const char* path)
{
    if (!isMounted)
        return nullptr;

    SPIBusLock::Guard guard;

    File f = SD.open(path);
    if (!f)
        return nullptr;

    return new ESP32SDFile(f);
}
