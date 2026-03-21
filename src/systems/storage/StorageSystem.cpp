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
    IFile* f = driver->open(path);
    if (!f) Log::error(Log::STORAGE_TAG, "Failed to open file: %s", path);
    else Log::debug(Log::STORAGE_TAG, "Opened file: %s", path);
    return f;
}

bool StorageSystem::readTextFile(const char* path, std::string& out)
{
    IFile* file = open(path);
    if (!file)
        return false;

    out.clear();
    char buffer[256];

    while (file->available())
    {
        int r = file->read(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
        if (r > 0)
            out.append(buffer, r);
        else
            break;
    }

    file->close();
    delete file;
    return true;
}
