#pragma once

#include <stdint.h>

class CpuMonitor {
  public:
    static volatile uint32_t tickCount[2];
    static volatile uint32_t idleCount[2];

    static void begin();
    static void update();

    static uint8_t usagePercentCore0();
    static uint8_t usagePercentCore1();
    static uint8_t usagePercentAvg();

  private:
    static uint8_t cpuUsage[2];
    static int64_t lastUpdateMs;
    static bool initialized;
};