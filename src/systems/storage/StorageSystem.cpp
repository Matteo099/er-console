#include "StorageSystem.h"

#include "drivers/storage/ESP32SDCardDriver.h"
#include "config/BoardConfig.h"
#include "log/Logger.h"
#include <SD.h>

namespace
{
    static void trimTrailingSlash(std::string& value)
    {
        while (!value.empty() && value.back() == '/')
            value.pop_back();
    }
}

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

bool StorageSystem::writeTextFile(const char* path, const std::string& contents, bool overwrite)
{
    if (!ready)
        return false;

    if (!ensureParents(path))
        return false;

    if (overwrite && SD.exists(path))
        SD.remove(path);

    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        Log::error(Log::STORAGE_TAG, "Failed to open file for write: %s", path);
        return false;
    }

    size_t written = file.print(contents.c_str());
    file.flush();
    file.close();
    return written == contents.size();
}

File StorageSystem::openNative(const char* path, bool write)
{
    if (!ready)
        return File();

    if (write && !ensureParents(path))
        return File();

    if (write && SD.exists(path))
        SD.remove(path);

    return SD.open(path, write ? FILE_WRITE : FILE_READ);
}

bool StorageSystem::removePath(const char* path)
{
    if (!ready)
        return false;

    return SD.remove(path);
}

bool StorageSystem::movePath(const char* sourcePath, const char* destinationPath, bool overwrite)
{
    if (!ready || !sourcePath || !destinationPath || !*sourcePath || !*destinationPath)
        return false;

    if (!SD.exists(sourcePath))
        return false;

    if (!ensureParents(destinationPath))
        return false;

    if (overwrite && SD.exists(destinationPath) && !SD.remove(destinationPath))
        return false;

    return SD.rename(sourcePath, destinationPath);
}

bool StorageSystem::createDirectory(const char* path)
{
    if (!ready || !path || !*path)
        return false;

    if (SD.exists(path))
        return true;

    if (!ensureParents(path))
        return false;

    return SD.mkdir(path);
}

bool StorageSystem::exists(const char* path) const
{
    return ready && path && SD.exists(path);
}

bool StorageSystem::listDirectory(const char* path, std::vector<StorageEntry>& out, bool recursive) const
{
    if (!ready)
        return false;

    File directory = SD.open(path);
    if (!directory || !directory.isDirectory())
        return false;

    while (true)
    {
        File entry = directory.openNextFile();
        if (!entry)
            break;

        StorageEntry info;
        info.path = std::string(path);
        trimTrailingSlash(info.path);
        info.path += "/";
        info.path += entry.name();
        info.name = entry.name();
        info.isDirectory = entry.isDirectory();
        info.size = entry.size();
        out.push_back(info);

        if (recursive && entry.isDirectory())
        {
            listDirectory(info.path.c_str(), out, true);
        }

        entry.close();
    }

    directory.close();
    return true;
}

uint64_t StorageSystem::totalBytes() const
{
    return ready ? SD.totalBytes() : 0;
}

uint64_t StorageSystem::usedBytes() const
{
    return ready ? SD.usedBytes() : 0;
}

uint64_t StorageSystem::freeBytes() const
{
    uint64_t total = totalBytes();
    uint64_t used = usedBytes();
    return total > used ? total - used : 0;
}

bool StorageSystem::ensureParents(const char* path)
{
    std::string parent = parentPath(path);
    if (parent.empty() || parent == "/")
        return true;

    if (SD.exists(parent.c_str()))
        return true;

    if (!ensureParents(parent.c_str()))
        return false;

    return SD.mkdir(parent.c_str());
}

std::string StorageSystem::parentPath(const char* path)
{
    if (!path)
        return {};

    std::string value(path);
    auto pos = value.find_last_of('/');
    if (pos == std::string::npos || pos == 0)
        return "/";

    return value.substr(0, pos);
}

std::string StorageSystem::fileName(const char* path)
{
    if (!path)
        return {};

    std::string value(path);
    auto pos = value.find_last_of('/');
    if (pos == std::string::npos)
        return value;

    return value.substr(pos + 1);
}
