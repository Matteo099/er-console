#include "StorageSystem.h"

#include "drivers/storage/ESP32SDCardDriver.h"
#include "drivers/storage/SDLock.h"
#include "config/BoardConfig.h"
#include "log/Logger.h"
#include <SD.h>
#include <config/Assets.h>

namespace
{
    static constexpr const char* SYSTEM_VOLUME_INFO_DIR = "System Volume Information";

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

    listFiles();
}

bool StorageSystem::isReady() const
{
    return ready;
}

void StorageSystem::listFiles() const
{
    if (!ready)
        return;

    Log::info(Log::STORAGE_TAG, "Path probe: /.core=%s /games=%s /brand.png=%s",
              exists(CORE_DIR) ? "yes" : "no",
              exists(GAMES_DIR) ? "yes" : "no",
              exists(IMG_BRAND_FILE) ? "yes" : "no");

    std::vector<StorageEntry> coreEntries;
    if (listDirectory(CORE_DIR, coreEntries, true))
    {
        Log::info(Log::STORAGE_TAG, "Listing files under /.core:");
        for (const auto& entry : coreEntries)
        {
            Log::info(Log::STORAGE_TAG, " - %s%s (size: %u bytes)", entry.path.c_str(), entry.isDirectory ? "/" : "", entry.size);
        }
    }

    std::vector<StorageEntry> gameEntries;
    if (listDirectory(GAMES_DIR, gameEntries, true))
    {
        Log::info(Log::STORAGE_TAG, "Listing files under /games:");
        for (const auto& entry : gameEntries)
        {
            Log::info(Log::STORAGE_TAG, " - %s%s (size: %u bytes)", entry.path.c_str(), entry.isDirectory ? "/" : "", entry.size);
        }
    }
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

    SDLock::Guard guard;

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

    SDLock::Guard guard;

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

    SDLock::Guard guard;

    return SD.remove(path);
}

bool StorageSystem::movePath(const char* sourcePath, const char* destinationPath, bool overwrite)
{
    if (!ready || !sourcePath || !destinationPath || !*sourcePath || !*destinationPath)
        return false;

    SDLock::Guard guard;

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

    SDLock::Guard guard;

    if (SD.exists(path))
        return true;

    if (!ensureParents(path))
        return false;

    return SD.mkdir(path);
}

bool StorageSystem::exists(const char* path) const
{
    if (!ready || !path)
        return false;

    SDLock::Guard guard;
    return SD.exists(path);
}

bool StorageSystem::listDirectory(const char* path, std::vector<StorageEntry>& out, bool recursive) const
{
    if (!ready)
        return false;

    SDLock::Guard guard;

    File directory = SD.open(path);
    if (!directory || !directory.isDirectory())
        return false;

    std::vector<std::string> childDirectories;

    while (true)
    {
        File entry = directory.openNextFile();
        if (!entry)
            break;

        const bool isDir = entry.isDirectory();
        const uint64_t size = entry.size();
        const std::string entryBaseName = fileName(entry.name());

        StorageEntry info;
        info.path = std::string(path);
        trimTrailingSlash(info.path);
        info.path += "/";
        info.path += entryBaseName;
        info.name = entryBaseName;
        info.isDirectory = isDir;
        info.size = size;
        out.push_back(info);

        if (recursive && isDir)
        {
            if (entryBaseName != SYSTEM_VOLUME_INFO_DIR)
            {
                childDirectories.push_back(info.path);
            }
        }

        entry.close();
    }

    directory.close();

    for (const auto& childPath : childDirectories)
    {
        if (!listDirectory(childPath.c_str(), out, true))
        {
            Log::warn(Log::STORAGE_TAG, "Failed to list subdirectory: %s", childPath.c_str());
        }
    }

    return true;
}

uint64_t StorageSystem::totalBytes() const
{
    if (!ready)
        return 0;

    SDLock::Guard guard;
    return SD.totalBytes();
}

uint64_t StorageSystem::usedBytes() const
{
    if (!ready)
        return 0;

    SDLock::Guard guard;
    return SD.usedBytes();
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
