#pragma once

#include <stdint.h>

struct BootEvent {
    uint8_t progress;
    const char* label;
};
