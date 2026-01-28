#include "StorageSystem.h"

#include "drivers/storage/ESP32SDCardDriver.h"
#include "config/BoardConfig.h"
#include "log/Logger.h"

StorageSystem::StorageSystem()
{
    driver = new ESP32SDCardDriver(SD_CS_PIN);
}

void StorageSystem::init(void *params)
{
    Log::info(Log::STORAGE_TAG, "Initializing storage");

    if (!driver->init())
    {
        Log::error(Log::STORAGE_TAG, "SD init failed");
        ready = false;
        return;
    }

    ready = true;
    Log::info(Log::STORAGE_TAG, "Storage ready");
}

bool StorageSystem::isReady() const
{
    return ready;
}

IFile* StorageSystem::open(const char* path)
{
    return driver->open(path);
}
