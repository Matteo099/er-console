#pragma once

#include "I2CBusManager.h"

class I2CBusLock
{
public:
    class Guard
    {
    public:
        Guard()
        {
            auto mutex = I2CBusManager::mutex();

            if (mutex)
            {
                xSemaphoreTake(
                    mutex,
                    portMAX_DELAY);
            }
        }

        ~Guard()
        {
            auto mutex = I2CBusManager::mutex();

            if (mutex)
            {
                xSemaphoreGive(
                    mutex);
            }
        }

        Guard(const Guard &) = delete;
        Guard &operator=(const Guard &) = delete;
    };
};