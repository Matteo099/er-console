#include "CpuMonitor.h"
#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint32_t CpuMonitor::lastSampleTime = 0;
uint32_t CpuMonitor::activeTime = 0;
uint8_t CpuMonitor::cpuUsage = 0;
bool CpuMonitor::isInitialized = false;

// ------------------------------
// Measurement task
// ------------------------------
static void cpuProbeTask(void *param)
{
    while (true)
    {
        CpuMonitor::activeTime++;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void CpuMonitor::begin()
{
    if (isInitialized)
        return;

    lastSampleTime = millis();
    activeTime = 0;
    cpuUsage = 0;

    xTaskCreatePinnedToCore(
        cpuProbeTask,
        "cpuProbe",
        2048,
        nullptr,
        1,
        nullptr,
        0);

    isInitialized = true;
}

uint8_t CpuMonitor::usagePercent()
{
    return cpuUsage;
}

void CpuMonitor::update()
{
    const uint32_t now = millis();

    const uint32_t elapsed = now - lastSampleTime;

    if (elapsed < 1000)
        return;

    lastSampleTime = now;

    // expected ticks in 1s window
    const uint32_t expected = 1000;

    // how many times probe task actually ran
    uint32_t measured = activeTime;

    activeTime = 0;

    // convert to % idle vs busy
    float load = 100.0f * (1.0f - (float)measured / expected);

    if (load < 0)
        load = 0;
    if (load > 100)
        load = 100;

    cpuUsage = (uint8_t)load;
}