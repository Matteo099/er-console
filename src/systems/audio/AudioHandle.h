#pragma once

#include <cstdint>

struct AudioHandle
{
    static constexpr uint16_t Invalid = 0;

    uint16_t id;

    AudioHandle(uint16_t value = Invalid)
        : id(value)
    {
    }

    bool valid() const
    {
        return id != Invalid;
    }

    void reset()
    {
        id = Invalid;
    }

    bool operator==(const AudioHandle &other) const
    {
        return id == other.id;
    }

    bool operator!=(const AudioHandle &other) const
    {
        return id != other.id;
    }
};