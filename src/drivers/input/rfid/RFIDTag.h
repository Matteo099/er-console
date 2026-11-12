#pragma once

#include <stdint.h>
#include <string>

struct RFIDTag {
    bool valid = false;

    std::string uid;

    uint32_t timestampMs = 0;
};