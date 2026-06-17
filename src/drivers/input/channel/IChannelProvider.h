#pragma once

#include <stdint.h>


class IChannelProvider
{
public:

    virtual ~IChannelProvider() = default;


    virtual bool init() = 0;


    virtual bool owns(
        uint16_t channel) const = 0;


    virtual void pinMode(
        uint16_t channel,
        uint8_t mode) = 0;


    virtual void digitalWrite(
        uint16_t channel,
        bool state) = 0;


    virtual bool digitalRead(
        uint16_t channel) = 0;


    virtual int analogRead(
        uint16_t channel) = 0;
};