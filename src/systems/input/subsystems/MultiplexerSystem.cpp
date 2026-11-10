#include "MultiplexerSystem.h"

void MultiplexerSystem::registerMultiplexer(
    IMultiplexer* mux)
{
    multiplexers.push_back(mux);
}

bool MultiplexerSystem::init()
{
    for(auto* mux : multiplexers)
    {
        if (!mux->init())
            return false;
    }

    return true;
}

void MultiplexerSystem::update()
{
}

void MultiplexerSystem::shutdown()
{
    for(auto* mux : multiplexers)
    {
        mux->shutdown();
    }
}

bool MultiplexerSystem::select(
    uint8_t mux,
    uint8_t channel)
{
    if(mux >= multiplexers.size())
        return false;

    return multiplexers[mux]
        ->selectChannel(channel);
}

int MultiplexerSystem::analogRead(
    uint8_t mux,
    uint8_t channel)
{
    if(mux >= multiplexers.size())
        return 0;

    return multiplexers[mux]
        ->analogRead(channel);
}

bool MultiplexerSystem::digitalRead(
    uint8_t mux,
    uint8_t channel)
{
    if(mux >= multiplexers.size())
        return false;

    return multiplexers[mux]
        ->digitalRead(channel);
}

void MultiplexerSystem::digitalWrite(
    uint8_t mux,
    uint8_t channel,
    bool value)
{
    if(mux >= multiplexers.size())
        return;

    multiplexers[mux]
        ->digitalWrite(
            channel,
            value);
}