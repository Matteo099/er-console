#pragma once

#include "SDCardDriver.h"
#include <SD.h>

class ESP32SDCardDriver : public SDCardDriver
{
public:
    ESP32SDCardDriver(uint8_t csPin);

    bool init() override;
    bool mounted() const override;

private:
    uint8_t cs;
    bool isMounted = false;
};
