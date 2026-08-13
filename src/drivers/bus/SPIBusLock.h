#pragma once

#include "SPIBusManager.h"

class SPIBusLock
{
public:
    class Guard
    {
    public:
        Guard()
            : mutex(SPIBusManager::mutex())
        {
            if (mutex)
            {
                xSemaphoreTake(
                    mutex,
                    portMAX_DELAY);
            }
        }

        ~Guard()
        {
            if (mutex)
            {
                xSemaphoreGive(mutex);
            }
        }

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

    private:
        SemaphoreHandle_t mutex;
    };
};