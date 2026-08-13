#include "SPIBusManager.h"

SPIClass SPIBusManager::spi = HSPI;

SemaphoreHandle_t SPIBusManager::busMutex = nullptr;

bool SPIBusManager::initialized = false;

bool SPIBusManager::init(int sclkPin, int misoPin, int mosiPin)
{
    if (initialized)
    {
        return true;
    }
    busMutex = xSemaphoreCreateMutex();
    if (!busMutex)
    {
        return false;
    }
    spi.begin(sclkPin, misoPin, mosiPin, -1); // CS handled by the device driver, so set to -1
    initialized = true;
    return true;
}

void SPIBusManager::shutdown()
{
    if (!initialized)
    {
        return;
    }
    spi.end();
    if (busMutex)
    {
        vSemaphoreDelete(busMutex);
        busMutex = nullptr;
    }
    initialized = false;
}

SPIClass &SPIBusManager::bus() { return spi; }

SemaphoreHandle_t SPIBusManager::mutex() { return busMutex; }