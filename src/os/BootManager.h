#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class BootManager {
public:
    enum class BootStep {
        CORE_INIT,
        DISPLAY_INIT,
        STORAGE_INIT,
        AUDIO_INIT,
        INPUT_INIT,
        LUA_INIT,
        DONE
    };

    void run();

    uint8_t getProgress() const;
    const char* getCurrentStepName() const;

private:
    void emitStep(BootStep step, uint8_t progress, const char* label);

    BootStep currentStep = BootStep::CORE_INIT;
    uint8_t progress = 0;
    const char* stepLabel = "Starting";
};