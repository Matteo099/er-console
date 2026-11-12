#pragma once

#include <string>
#include <cstdint>
#include "systems/input/InputConfig.h"

struct GameInfo {
    std::string id;
    std::string title;
    std::string description;
    std::string author;
    std::string englishTitle;
    std::string englishDescription;

    const std::string& titleFor(bool english) const {
        return english && !englishTitle.empty() ? englishTitle : title;
    }

    const std::string& descriptionFor(bool english) const {
        return english && !englishDescription.empty() ? englishDescription : description;
    }

    uint8_t difficulty;
    uint16_t durationMin;
    InputConfig inputConfig;
};
