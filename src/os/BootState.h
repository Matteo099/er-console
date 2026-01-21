#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct BootState {
    uint8_t progress;
    const char* label;
};

extern BootState gBootState;
extern SemaphoreHandle_t gBootStateMutex;
