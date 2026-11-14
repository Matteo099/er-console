#include "I2CBusManager.h"

TwoWire I2CBusManager::wire = TwoWire(0);

SemaphoreHandle_t I2CBusManager::busMutex = nullptr;

bool I2CBusManager::initialized = false;

bool I2CBusManager::init(int sdaPin, int sclPin, uint32_t frequency) {
    if (initialized) {
        return true;
    }
    busMutex = xSemaphoreCreateMutex();
    if (!busMutex) {
        return false;
    }
    if (!wire.begin(sdaPin, sclPin, frequency)) {
        vSemaphoreDelete(busMutex);
        busMutex = nullptr;
        return false;
    }
    wire.setTimeOut(20); // Bound a missing/stuck room device without blocking the UI.
    initialized = true;
    return true;
}

void I2CBusManager::shutdown() {
    if (!initialized) {
        return;
    }
    wire.end();
    if (busMutex) {
        vSemaphoreDelete(busMutex);
        busMutex = nullptr;
    }
    initialized = false;
}

TwoWire& I2CBusManager::bus() {
    return wire;
}

SemaphoreHandle_t I2CBusManager::mutex() {
    return busMutex;
}
