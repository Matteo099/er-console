#include "ESP32SDCardDriver.h"
#include "log/Logger.h"
#include "drivers/bus/SPIBusLock.h"
#include "config/BoardConfig.h"
#include "ESP32SDFile.h"
#include <cstring>

namespace
{
    static constexpr const char *SYSTEM_VOLUME_INFO_DIR =
        "System Volume Information";

    static std::string fileName(const char *path)
    {
        if (!path)
            return {};

        std::string value(path);
        auto pos = value.find_last_of('/');

        if (pos == std::string::npos)
            return value;

        return value.substr(pos + 1);
    }

    static void trimTrailingSlash(std::string &value)
    {
        while (!value.empty() && value.back() == '/')
            value.pop_back();
    }
}

ESP32SDCardDriver::ESP32SDCardDriver(SPIClass &spi, uint8_t csPin)
    : spi(spi), cs(csPin) {}

bool ESP32SDCardDriver::init()
{
    SPIBusLock::Guard guard;

    Log::info(Log::DRIVER_TAG, "Initializing SD Card (CS=%d)", cs);
    Log::info(Log::DRIVER_TAG, "SD SPI clock: %lu Hz", (unsigned long)SD_SPI_CLOCK);
    isMounted = SD.begin(cs, spi, SD_SPI_CLOCK, "/sd");
    return isMounted;
}

bool ESP32SDCardDriver::mounted() const
{
    return isMounted;
}

IFile *ESP32SDCardDriver::open(const char *path, FileMode mode)
{
    if (!isMounted)
        return nullptr;

    SPIBusLock::Guard guard;
    File file;

    switch (mode)
    {
    case FileMode::Read:
        file = SD.open(path, FILE_READ);
        break;
    case FileMode::Write:
        file = SD.open(path, FILE_WRITE);
        break;
    case FileMode::Append:
        file = SD.open(path, FILE_APPEND);
        break;
    }

    if (!file)
        return nullptr;

    return new ESP32SDFile(file);
}

File ESP32SDCardDriver::openNative(const char *path, bool write)
{
    if (!isMounted || !path)
        return File();

    SPIBusLock::Guard guard;

    if (write && !ensureParents(path))
        return File();

    if (write && SD.exists(path))
        SD.remove(path);

    return SD.open(path, write ? FILE_WRITE : FILE_READ);
}

bool ESP32SDCardDriver::exists(const char *path) const
{
    if (!isMounted || !path)
        return false;

    SPIBusLock::Guard guard;

    return SD.exists(path);
}

bool ESP32SDCardDriver::remove(const char *path)
{
    if (!isMounted || !path)
        return false;

    SPIBusLock::Guard guard;

    return SD.remove(path);
}

bool ESP32SDCardDriver::rename(
    const char *src,
    const char *dst)
{
    if (!isMounted || !src || !dst)
        return false;

    SPIBusLock::Guard guard;

    return SD.rename(src, dst);
}

bool ESP32SDCardDriver::mkdir(const char *path)
{
    if (!isMounted || !path)
        return false;

    SPIBusLock::Guard guard;

    if (SD.exists(path))
        return true;

    return SD.mkdir(path);
}

bool ESP32SDCardDriver::listDirectory(
    const char *path,
    std::vector<StorageEntry> &out) const
{
    if (!isMounted || !path)
        return false;

    SPIBusLock::Guard guard;

    File directory = SD.open(path);

    if (!directory || !directory.isDirectory())
        return false;

    while (true)
    {
        File entry = directory.openNextFile();

        if (!entry)
            break;

        StorageEntry info;

        const bool isDir = entry.isDirectory();

        std::string name = fileName(entry.name());

        info.name = name;

        if (std::strcmp(path, "/") == 0)
        {
            info.path = "/" + name;
        }
        else
        {
            info.path = std::string(path);

            if (!info.path.empty() && info.path.back() != '/')
                info.path += '/';

            info.path += name;
        }
        info.name = fileName(entry.name());
        info.isDirectory = isDir;
        info.size = entry.size();

        out.push_back(std::move(info));

        entry.close();
    }

    directory.close();

    return true;
}

uint64_t ESP32SDCardDriver::totalBytes() const
{
    if (!isMounted)
        return 0;

    SPIBusLock::Guard guard;

    return SD.totalBytes();
}

uint64_t ESP32SDCardDriver::usedBytes() const
{
    if (!isMounted)
        return 0;

    SPIBusLock::Guard guard;

    return SD.usedBytes();
}

bool ESP32SDCardDriver::ensureParents(const char *path)
{
    std::string parent = parentPath(path);

    if (parent.empty() ||
        parent == "/")
    {
        return true;
    }

    if (exists(parent.c_str()))
        return true;

    if (!ensureParents(parent.c_str()))
        return false;

    return mkdir(parent.c_str());
}

std::string ESP32SDCardDriver::parentPath(
    const char *path)
{
    if (!path)
        return {};

    std::string value(path);

    auto pos =
        value.find_last_of('/');

    if (pos == std::string::npos ||
        pos == 0)
    {
        return "/";
    }

    return value.substr(0, pos);
}
