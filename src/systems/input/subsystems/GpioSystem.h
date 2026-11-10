#pragma once

#include <vector>

#include "ISubsystem.h"
#include "drivers/input/gpio/IGpioProvider.h"

class GpioSystem : public ISubsystem
{
public:
    void registerProvider(IGpioProvider *provider);
    bool init() override;
    void update() override;
    void shutdown() override;
    void pinMode(uint16_t pin, uint8_t mode);
    void digitalWrite(uint16_t pin, bool state);
    bool digitalRead(uint16_t pin);

private:
    IGpioProvider *providerFor(uint16_t pin);

private:
    std::vector<IGpioProvider *> providers;
};