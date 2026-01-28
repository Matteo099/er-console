#pragma once

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/audio/AudioDriver.h"

class AudioSystem : public SystemService
{
public:
    AudioSystem();

    void init(void *params = nullptr) override;
    void update() override;

    void beep(uint16_t freq = 1000, uint16_t ms = 100);

private:
    StorageSystem *storage;
    AudioDriver *driver;
    uint8_t volume = 80;
};
