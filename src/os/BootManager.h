#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class BootManager
{
public:
    enum class BootStep : uint8_t
    {
        CORE_INIT = 0,
        DISPLAY_INIT,
        STORAGE_INIT,
        AUDIO_INIT,
        INPUT_INIT,
        LUA_INIT,

        COUNT,
        DONE = COUNT
    };

    void run();

private:
    uint8_t bootProgress(BootStep step);
    void emitStep(BootStep step, const char *label);
};