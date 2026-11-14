#include "MonitorScene.h"

#include <Arduino.h>
#include <cstdio>

#include "core/SystemEventBus.h"
#include "systems/ui/UIEvents.h"
#include "systems/input/InputSystem.h"
#include "systems/storage/StorageSystem.h"
#include "core/CpuMonitor.h"

void MonitorScene::onEnter() {
    view.setEventHandler(this);
    view.create();

    CpuMonitor::begin();

    input = static_cast<InputSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
    storage = static_cast<StorageSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    if (storage)
        storage->requestSpaceUsage();

    refreshTelemetry();
}

void MonitorScene::onUpdate() {
    if (millis() - lastRefreshMs >= 1000)
        refreshTelemetry();
    view.update();
}

void MonitorScene::onExit() {
    view.destroy();
}

SceneRequest MonitorScene::getRequest() const {
    return request;
}

bool MonitorScene::handleUIEvent(const UIEvent& event) {
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "refresh_sd") {
        if (storage)
            storage->requestSpaceUsage();
        view.setSdUsagePending(false);
        return true;
    }
    if (clicked.buttonId == "back") {
        request = SceneRequest(SceneChange::GoToSettingsSystem, SceneTransition::SlideRight);
        return true;
    }

    return false;
}

void MonitorScene::refreshTelemetry() {
    lastRefreshMs = millis();
    //
    // Power
    //
    PowerReading power;
    if (!input || !input->powerSnapshot(power)) {
        view.setPowerAvailable(false);
    } else {
        view.setPowerAvailable(true);
        view.setPower(power.loadVoltageV, power.currentMa, power.powerMw);
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
    // TOTAL RAM (Heap + PSRAM)
    //
    const uint32_t usedRam = usedHeap + usedPsram;
    const uint32_t totalRam = totalHeap + totalPsram;
    view.setRamUsage(usedRam / 1024, totalRam / 1024);

    //
    // SD Card
    //
    if (!storage || !storage->isReady()) {
        view.setSdCapacity(0);
        return;
    }

    view.setSdCapacity(static_cast<uint32_t>(storage->capacityBytes() / (1024ULL * 1024ULL)));
    const auto usage = storage->spaceUsage();
    if (usage.state == StorageSystem::SpaceUsage::State::Ready)
        view.setSdUsage(usage.total, usage.used);
    else
        view.setSdUsagePending(usage.state == StorageSystem::SpaceUsage::State::Failed);
}
