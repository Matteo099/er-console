#pragma once

#include <Adafruit_NeoPixel.h>
#include "core/SystemService.h"
#include "config/BoardConfig.h"

class RainbowPixel : public SystemService
{
private:
    Adafruit_NeoPixel pixels;

    uint32_t wheel(byte WheelPos);

public:
    RainbowPixel();

    void init(void *params = nullptr) override;
    void update() override;
};
