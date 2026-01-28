#pragma once

#include <SD.h>
#include "SDCardDriver.h"
#include "drivers/storage/IFile.h"

class ESP32SDCardDriver : public SDCardDriver
{
public:
    ESP32SDCardDriver(uint8_t csPin);

    bool init() override;
    bool mounted() const override;
    IFile* open(const char* path) override;

private:
    uint8_t cs;
    bool isMounted = false;
};
