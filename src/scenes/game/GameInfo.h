#pragma once

#include <string>
#include <cstdint>

struct GameInfo {
    std::string id;
    std::string title;
    std::string description;
    std::string author;
    uint8_t difficulty;
    uint16_t durationMin;
};