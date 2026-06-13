#include "CpuMonitor.h"
#include "esp_freertos_hooks.h"
#include <Arduino.h>

// ------------------------------
// Static storage
// ------------------------------
volatile uint32_t CpuMonitor::tickCount[2] = {0, 0};
volatile uint32_t CpuMonitor::idleCount[2] = {0, 0};

uint8_t CpuMonitor::cpuUsage[2] = {0, 0};
uint32_t CpuMonitor::lastUpdateMs = 0;
bool CpuMonitor::initialized = false;

// ------------------------------
// FreeRTOS hooks
// ------------------------------
static bool idleHook0()
{
    CpuMonitor::idleCount[0]++;
    return true;
}

static bool idleHook1()
{
    CpuMonitor::idleCount[1]++;
    return true;
}

static void tickHook0()
{
    CpuMonitor::tickCount[0]++;
}

static void tickHook1()
{
    CpuMonitor::tickCount[1]++;
}

// ------------------------------
// Init
// ------------------------------
void CpuMonitor::begin()
{
    if(initialized)
        return;
        
    lastUpdateMs = millis();

    esp_register_freertos_idle_hook_for_cpu(idleHook0, 0);
    esp_register_freertos_idle_hook_for_cpu(idleHook1, 1);

    esp_register_freertos_tick_hook_for_cpu(tickHook0, 0);
    esp_register_freertos_tick_hook_for_cpu(tickHook1, 1);

    initialized = true;
}

void CpuMonitor::update()
{
    const uint32_t now = millis();

    if (now - lastUpdateMs < 1000)
        return;

    lastUpdateMs = now;

    for (int c = 0; c < 2; c++)
    {
        uint32_t ticks = tickCount[c];
        uint32_t idle = idleCount[c];

        // reset counters
        tickCount[c] = 0;
        idleCount[c] = 0;

        float load = 0;

        if (ticks > 0)
        {
            float idleRatio = (float)idle / (float)ticks;
            load = 100.0f * (1.0f - idleRatio);
        }

        if (load < 0) load = 0;
        if (load > 100) load = 100;

        cpuUsage[c] = (uint8_t)load;
    }
}

uint8_t CpuMonitor::usagePercentCore0()
{
    return cpuUsage[0];
}

uint8_t CpuMonitor::usagePercentCore1()
{
    return cpuUsage[1];
}

uint8_t CpuMonitor::usagePercentAvg()
{
    return (cpuUsage[0] + cpuUsage[1]) / 2;
}