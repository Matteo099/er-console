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
                xSemaphoreTakeRecursive(
                    mutex,
                    portMAX_DELAY);
            }
        }

        ~Guard()
        {
            if (mutex)
            {
                xSemaphoreGiveRecursive(mutex);
            }
        }

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;

    private:
        SemaphoreHandle_t mutex;
    };
};
