#pragma once

#include <stdint.h>


struct RelayState
{
    bool enabled = false;

    bool activeLow = false;

    uint32_t changedAt = 0;
};