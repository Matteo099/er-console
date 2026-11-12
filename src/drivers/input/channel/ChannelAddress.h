#pragma once

#include <stdint.h>

enum ChannelType { GPIO, MULTIPLEXED };

struct ChannelAddress {
    ChannelType type;
    uint16_t id;
    uint8_t device;
    uint8_t channel;
};