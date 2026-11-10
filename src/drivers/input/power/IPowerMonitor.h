#pragma once

#include <stdint.h>

struct PowerReading
{
    bool available = false;
    bool overflow = false;

    float busVoltageV = 0.0f;
    float shuntVoltageMv = 0.0f;
    float loadVoltageV = 0.0f;

    float currentMa = 0.0f;
    float powerMw = 0.0f;

    uint32_t sampleTimeMs = 0;
};

class IPowerMonitor
{
public:
    virtual ~IPowerMonitor() = default;

    virtual bool init() = 0;

    virtual void shutdown() = 0;

    virtual bool read(PowerReading& reading) = 0;

    virtual bool isAvailable() const = 0;
};