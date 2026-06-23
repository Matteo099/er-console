#pragma once

#include "SPIBusManager.h"

class SPIBusLock
{

public:
    class Guard
    {

    public:
        Guard()
        {
            auto m = SPIBusManager::mutex();

            if (m)
            {
                xSemaphoreTake(
                    m,
                    portMAX_DELAY);
            }
        }

        ~Guard()
        {
            auto m = SPIBusManager::mutex();

            if (m)
            {
                xSemaphoreGive(m);
            }
        }

        Guard(const Guard &) = delete;

        Guard &operator=(const Guard &) = delete;
    };
};