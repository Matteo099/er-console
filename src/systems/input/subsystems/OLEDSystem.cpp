#include "OLEDSystem.h"

#include "log/Logger.h"

OLEDSystem::OLEDSystem(IDisplay& display) : display(display) {
}

bool OLEDSystem::init() {
    supported = display.init();
    if (!supported) {
        Log::warn(Log::INPUT_TAG, "OLED display unavailable");
    } else {
        Log::info(Log::INPUT_TAG, "OLED display initialized");
    }
    return supported;
}

void OLEDSystem::update() {
    if (!supported)
        return;
    portENTER_CRITICAL(&lock);
    const bool redraw = dirty;
    const std::string text = message;
    dirty = false;
    portEXIT_CRITICAL(&lock);
    if (!redraw)
        return;
    display.clear();
    display.print(text);
    display.refresh();
}

void OLEDSystem::shutdown() {
    if (supported) {
        display.shutdown();
        supported = false;
        Log::info(Log::INPUT_TAG, "OLED display shutdown");
    }
}

bool OLEDSystem::isSupported() const {
    return supported;
}

void OLEDSystem::clear() {
    portENTER_CRITICAL(&lock);
    message.clear();
    dirty = true;
    portEXIT_CRITICAL(&lock);
}

void OLEDSystem::showMessage(const std::string& text) {
    portENTER_CRITICAL(&lock);
    message = text;
    dirty = true;
    portEXIT_CRITICAL(&lock);
}

std::string OLEDSystem::currentMessage() const {
    portENTER_CRITICAL(&lock);
    std::string copy = message;
    portEXIT_CRITICAL(&lock);
    return copy;
}