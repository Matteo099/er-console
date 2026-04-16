#include "MonitorScene.h"

#include <Arduino.h>
#include <cstdio>

#include "core/SystemEventBus.h"
#include "systems/ui/UIEvents.h"
#include "systems/input/InputSystem.h"
#include "systems/storage/StorageSystem.h"

void MonitorScene::onEnter()
{
    view.setEventHandler(this);
    view.create();

    input = static_cast<InputSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
    storage = static_cast<StorageSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));

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

bool MonitorScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToSettings);
        return true;
    }

    return false;
}

void MonitorScene::refreshTelemetry()
{
    char line1[96];
    char line2[96];
    char line3[96];

    if (!input)
    {
        view.setPowerStatus("Input service unavailable", "-", "-");
    }
    else
    {
        const auto power = input->getPowerMetrics();
        if (!power.supported)
        {
            view.setPowerStatus("INA219 not detected", "-", "-");
        }
        else if (!power.available)
        {
            view.setPowerStatus("Power monitor active", "Waiting sample...", "-");
        }
        else
        {
            std::snprintf(line1, sizeof(line1), "Bus %.2fV | Load %.2fV", power.busVoltageV, power.loadVoltageV);
            std::snprintf(line2, sizeof(line2), "Current %.1fmA | Power %.1fmW", power.currentMa, power.powerMw);
            std::snprintf(line3, sizeof(line3), "Shunt %.2fmV", power.shuntVoltageMv);
            view.setPowerStatus(line1, line2, line3);
        }
    }

    std::snprintf(line1, sizeof(line1), "CPU %lu MHz | Uptime %lus", ESP.getCpuFreqMHz(), millis() / 1000UL);
    std::snprintf(line2, sizeof(line2), "Heap %s", formatBytes(ESP.getFreeHeap()));
    std::snprintf(line3, sizeof(line3), "PSRAM %s", formatBytes(ESP.getFreePsram()));
    view.setEspStatus(line1, line2, line3);

    if (!storage || !storage->isReady())
    {
        view.setSdStatus("SD not ready", "-", "-");
    }
    else
    {
        const uint64_t total = storage->totalBytes();
        const uint64_t used = storage->usedBytes();
        const uint64_t free = storage->freeBytes();

        std::snprintf(line1, sizeof(line1), "Total %s", formatBytes(total));
        std::snprintf(line2, sizeof(line2), "Used %s", formatBytes(used));
        std::snprintf(line3, sizeof(line3), "Free %s", formatBytes(free));
        view.setSdStatus(line1, line2, line3);
    }
}

const char* MonitorScene::formatBytes(uint64_t bytes)
{
    static char buffer[32];
    const char* units[] = {"B", "KB", "MB", "GB"};
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
