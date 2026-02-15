#include "ESP32SDCardDriver.h"
#include "log/Logger.h"

#include "ESP32SDFile.h"

ESP32SDCardDriver::ESP32SDCardDriver(uint8_t csPin)
    : cs(csPin) {}

bool ESP32SDCardDriver::init()
{
    Log::info(Log::DRIVER_TAG, "Initializing SD Card (CS=%d)", cs);
    // isMounted = SD.begin(cs);
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

    File f = SD.open(path);
    if (!f)
        return nullptr;

    return new ESP32SDFile(f);
}
