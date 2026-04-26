#include "SDLock.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace
{
    SemaphoreHandle_t g_sdMutex = nullptr;

    SemaphoreHandle_t ensureMutex()
    {
        if (!g_sdMutex)
        {
            g_sdMutex = xSemaphoreCreateRecursiveMutex();
        }
        return g_sdMutex;
    }
}

void SDLock::init()
{
    (void)ensureMutex();
}

void SDLock::lock()
{
    SemaphoreHandle_t mutex = ensureMutex();
    if (mutex)
    {
        xSemaphoreTakeRecursive(mutex, portMAX_DELAY);
    }
}

void SDLock::unlock()
{
    if (g_sdMutex)
    {
        xSemaphoreGiveRecursive(g_sdMutex);
    }
}

SDLock::Guard::Guard()
{
    SDLock::lock();
}

SDLock::Guard::~Guard()
{
    SDLock::unlock();
}
