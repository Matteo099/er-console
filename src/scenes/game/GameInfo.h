#pragma once

#include <string>
#include <cstdint>
#include "systems/input/InputConfig.h"

struct GameInfo {
    std::string id;
    std::string title;
    std::string description;
    std::string author;
    uint8_t difficulty;
    uint16_t durationMin;
    InputConfig inputConfig;
};