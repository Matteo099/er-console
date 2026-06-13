#include "MonitorScene.h"

#include <Arduino.h>
#include <cstdio>

#include "core/SystemEventBus.h"
#include "systems/ui/UIEvents.h"
#include "systems/input/InputSystem.h"
#include "systems/storage/StorageSystem.h"
#include "core/CpuMonitor.h"

void MonitorScene::onEnter()
{
    view.setEventHandler(this);
    view.create();

    CpuMonitor::begin();

    input = static_cast<InputSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
    storage = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));

    refreshTelemetry();
}

void MonitorScene::onUpdate()
{
    refreshTelemetry();
    view.update();
}

void MonitorScene::onExit()
{
    view.destroy();
}

SceneRequest MonitorScene::getRequest() const
{
    return request;
}

bool MonitorScene::handleUIEvent(const UIEvent &event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto &clicked = static_cast<const ButtonClickedEvent &>(event);
    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToSettings, SceneTransition::SlideRight);
        return true;
    }

    return false;
}

void MonitorScene::refreshTelemetry()
{
    //
    // Power
    //
    if (!input)
    {
        view.setPower(0, 0, 0);
    }
    else
    {
        const auto power = input->getPowerMetrics();
        if (!power.supported || !power.available)
        {
            view.setPower(0, 0, 0);
        }
        else
        {
            view.setPower(power.loadVoltageV, power.currentMa, power.powerMw);
        }
    }

    //
    // ESP32
    //
    char uptime[32];
    const uint32_t uptimeSec = millis() / 1000UL;
    const uint32_t hours = uptimeSec / 3600UL;
    const uint32_t mins = (uptimeSec % 3600UL) / 60UL;
    const uint32_t secs = uptimeSec % 60UL;
    std::snprintf(uptime, sizeof(uptime), "Uptime %luh %lum %lus", hours, mins, secs);
    view.setUptime(uptime);

    //
    // CPU
    //
    CpuMonitor::update();
    view.setCpuUsage(CpuMonitor::usagePercentAvg());

    //
    // Heap
    //
    const uint32_t freeHeap = ESP.getFreeHeap();
    const uint32_t totalHeap = ESP.getHeapSize();
    const uint32_t usedHeap = totalHeap - freeHeap;
    view.setHeap(usedHeap / 1024, totalHeap / 1024);

    //
    // PSRAM
    //
    const uint32_t totalPsram = ESP.getPsramSize();
    const uint32_t freePsram = ESP.getFreePsram();
    const uint32_t usedPsram = totalPsram > freePsram ? totalPsram - freePsram : 0;
    view.setPsram(usedPsram / 1024, totalPsram / 1024);

    //
    // SD Card
    //
    if (!storage || !storage->isReady())
    {
        view.setSdStorage(0, 0);
        return;
    }

    const uint64_t totalBytes = storage->totalBytes();
    const uint64_t usedBytes = storage->usedBytes();
    view.setSdStorage(static_cast<uint32_t>(usedBytes / (1024ULL * 1024ULL)), static_cast<uint32_t>(totalBytes / (1024ULL * 1024ULL)));
}

const char *MonitorScene::formatBytes(uint64_t bytes)
{
    static char buffer[32];
    const char *units[] = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes);
    int unit = 0;

    while (value >= 1024.0 && unit < 3)
    {
        value /= 1024.0;
        ++unit;
    }

    if (unit == 0)
        std::snprintf(buffer, sizeof(buffer), "%llu %s", static_cast<unsigned long long>(bytes), units[unit]);
    else
        std::snprintf(buffer, sizeof(buffer), "%.2f %s", value, units[unit]);

    return buffer;
}
