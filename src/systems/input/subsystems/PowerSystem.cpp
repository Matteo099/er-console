#include "PowerSystem.h"

#include <Arduino.h>

#include "log/Logger.h"

PowerSystem::PowerSystem(IPowerMonitor& monitor) : monitor(monitor) {
}

bool PowerSystem::init() {
    supported = monitor.init();
    if (!supported) {
        Log::warn(Log::INPUT_TAG, "Power monitor unavailable");
    } else {
        Log::info(Log::INPUT_TAG, "Power monitor initialized");
    }
    return supported;
}

void PowerSystem::update() {
    poll();
}

void PowerSystem::shutdown() {
    if (supported) {
        monitor.shutdown();
        supported = false;
        Log::info(Log::INPUT_TAG, "Power monitor shutdown");
    }
}

bool PowerSystem::isSupported() const {
    return supported;
}

PowerReading PowerSystem::getMetrics() const {
    portENTER_CRITICAL(&lock);
    PowerReading copy = metrics;
    portEXIT_CRITICAL(&lock);
    return copy;
}

void PowerSystem::poll() {
    if (!supported)
        return;
    const uint32_t now = millis();
    if (now - lastPollMs < POLL_INTERVAL_MS)
        return;
    lastPollMs = now;
    PowerReading reading;
    if (!monitor.read(reading))
        return;
    portENTER_CRITICAL(&lock);
    metrics = reading;
    portEXIT_CRITICAL(&lock);
}