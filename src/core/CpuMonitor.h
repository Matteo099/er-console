#pragma once

#include <stdint.h>

class CpuMonitor
{
public:
    static void begin();
    static void update();
    static uint8_t usagePercent();

    static uint32_t activeTime;

private:
    static uint32_t lastSampleTime;
    static uint8_t cpuUsage;
    static bool isInitialized;
};