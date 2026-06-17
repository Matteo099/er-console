#pragma once

#include "drivers/input/power/IPowerMonitor.h"
#include "ISubsystem.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

class PowerSystem : public ISubsystem
{
public:
    explicit PowerSystem(IPowerMonitor& monitor);

    bool init() override;

    void update() override;

    PowerReading getMetrics() const;

    bool isSupported() const;

private:
    void poll();

private:
    IPowerMonitor& monitor;

    mutable portMUX_TYPE lock =
        portMUX_INITIALIZER_UNLOCKED;

    PowerReading metrics;

    bool supported = false;

    uint32_t lastPollMs = 0;

    static constexpr uint32_t POLL_INTERVAL_MS = 500;
};