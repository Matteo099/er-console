#pragma once

#include <optional>

#include "drivers/input/rfid/IRFIDReader.h"
#include "ISubsystem.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

class RFIDSystem : public ISubsystem
{
public:
    explicit RFIDSystem(IRFIDReader &reader);

    bool init() override;

    void update() override;

    bool isSupported() const;

    bool getLastTag(RFIDTag &tag) const;

    bool consumeTag(RFIDTag &tag);

private:
    void poll();

private:
    IRFIDReader &reader;

    mutable portMUX_TYPE lock =
        portMUX_INITIALIZER_UNLOCKED;

    bool supported = false;

    bool newTagAvailable = false;

    RFIDTag lastTag;

    uint32_t lastPollMs = 0;

    static constexpr uint32_t POLL_INTERVAL_MS = 50;

    static constexpr uint32_t DUPLICATE_TIMEOUT_MS = 2000;
};