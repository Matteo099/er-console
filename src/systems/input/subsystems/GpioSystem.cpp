#include "GpioSystem.h"

void GpioSystem::registerProvider(IGpioProvider *provider) { providers.push_back(provider); }

bool GpioSystem::init()
{
    for (auto *provider : providers)
        if (!provider->init())
            return false;
    return true;
}

void GpioSystem::update() {}

IGpioProvider *GpioSystem::providerFor(uint16_t pin)
{
    for (auto *provider : providers)
    {
        if (provider->ownsPin(pin))
            return provider;
    }
    return nullptr;
}

void GpioSystem::pinMode(uint16_t pin, uint8_t mode)
{
    auto *provider = providerFor(pin);
    if (provider)
        provider->pinMode(pin, mode);
}

void GpioSystem::digitalWrite(uint16_t pin, bool state)
{
    auto *provider = providerFor(pin);
    if (provider)
        provider->digitalWrite(pin, state);
}

bool GpioSystem::digitalRead(uint16_t pin)
{
    auto *provider = providerFor(pin);
    if (!provider)
        return false;
    return provider->digitalRead(pin);
}