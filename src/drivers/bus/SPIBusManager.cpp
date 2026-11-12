#include "SPIBusManager.h"
#include "log/Logger.h"

// Arduino-ESP32 2.x on ESP32-S3: HSPI maps to SPI3, FSPI to SPI2.
// TFT and touch use SPI2_HOST through LovyanGFX.
SPIClass SPIBusManager::spi = HSPI;

SemaphoreHandle_t SPIBusManager::busMutex = nullptr;

bool SPIBusManager::initialized = false;

bool SPIBusManager::init(int sclkPin, int misoPin, int mosiPin) {
    if (initialized) {
        return true;
    }
    // The SD driver can call methods which acquire the bus lock again
    // while the caller is holding it for a complete filesystem operation.
    // Use a recursive mutex so that this is safe and does not deadlock.
    busMutex = xSemaphoreCreateRecursiveMutex();
    if (!busMutex) {
        return false;
    }
    spi.begin(sclkPin, misoPin, mosiPin, -1);  // CS handled by the device driver, so set to -1
    if (!spi.bus()) {
        vSemaphoreDelete(busMutex);
        busMutex = nullptr;
        return false;
    }
    Log::info(Log::DRIVER_TAG, "SD bus: SPI3 (Arduino HSPI), SCK=%d MISO=%d MOSI=%d", sclkPin,
              misoPin, mosiPin);
    initialized = true;
    return true;
}

void SPIBusManager::shutdown() {
    if (!initialized) {
        return;
    }
    spi.end();
    if (busMutex) {
        vSemaphoreDelete(busMutex);
        busMutex = nullptr;
    }
    initialized = false;
}

SPIClass& SPIBusManager::bus() {
    return spi;
}

SemaphoreHandle_t SPIBusManager::mutex() {
    return busMutex;
}
