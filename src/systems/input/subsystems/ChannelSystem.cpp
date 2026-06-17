#include "ChannelSystem.h"

void ChannelSystem::registerProvider(
    IChannelProvider *provider)
{
    providers.push_back(provider);
}

bool ChannelSystem::init()
{
    for (auto p : providers)
        p->init();

    return true;
}

void ChannelSystem::update()
{
}

IChannelProvider *
ChannelSystem::findProvider(
    uint16_t channel)
{

    for (auto p : providers)
    {
        if (p->owns(channel))
            return p;
    }

    return nullptr;
}

void ChannelSystem::pinMode(
    uint16_t channel,
    uint8_t mode)
{
    auto p = findProvider(channel);

    if (p)
        p->pinMode(
            channel,
            mode);
}

void ChannelSystem::digitalWrite(
    uint16_t channel,
    bool state)
{
    auto p = findProvider(channel);

    if (p)
        p->digitalWrite(
            channel,
            state);
}

bool ChannelSystem::digitalRead(
    uint16_t channel)
{
    auto p = findProvider(channel);

    if (!p)
        return false;

    return p->digitalRead(channel);
}

int ChannelSystem::analogRead(
    uint16_t channel)
{

    auto p = findProvider(channel);

    if (!p)
        return 0;

    return p->analogRead(channel);
}