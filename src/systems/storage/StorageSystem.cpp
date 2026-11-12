#include "StorageSystem.h"

#include "drivers/storage/ESP32SDCardDriver.h"
#include "drivers/bus/SPIBusManager.h"
#include "drivers/bus/SPIBusLock.h"
#include "config/BoardConfig.h"
#include "config/Assets.h"
#include "log/Logger.h"
#include "services/GameAssetCache.h"

#include <algorithm>
#include <vector>
#include <cerrno>

namespace {
static constexpr const char* SYSTEM_VOLUME_INFO_DIR = "System Volume Information";
}

StorageSystem::StorageSystem() {
}

void StorageSystem::init(void* params) {
    Log::info(Log::STORAGE_TAG, "Initializing storage");

    auto& spi = SPIBusManager::bus();
    driver = new ESP32SDCardDriver(spi, SD_CS_PIN);
#if RECOVERY_STORAGE_SYSTEM_ENABLED
    recovery = new StorageRecoverySystem(driver, SD_POWER_RELAY_PIN, true);
#endif

    if (!driver->init()) {
        Log::error(Log::STORAGE_TAG, "SD init failed");
#if RECOVERY_STORAGE_SYSTEM_ENABLED
        if (!recovery->recover(StorageFailureReason::MountFailed)) {
            ready = false;
            return;
        }
#else
        ready = false;
        return;
#endif
    }

    cardCapacity = driver->capacityBytes();
    ready = true;
    noteIOSuccess();
    Log::info(Log::STORAGE_TAG, "Storage ready");

#if SD_READ_DIAGNOSTICS_ENABLED
    runReadDiagnostics("storage init");
#endif
#if STORAGE_BOOT_LIST_ENABLED
    listFiles();
#endif
}

#if SD_READ_DIAGNOSTICS_ENABLED
void StorageSystem::runReadDiagnostics(const char* phase) const {
    SPIBusLock::Guard guard;
    if (!ready) {
        Log::warn(Log::STORAGE_TAG, "SD diagnostic (%s): skipped, mount failed", phase);
        return;
    }

    const char* paths[] = {IMG_BRAND_NEGATIVE_FILE, IMG_LOGO_FILE};
    uint8_t buffer[512];
    for (const char* path : paths) {
        uint32_t firstHash = 0;
        size_t firstSize = 0;
        for (unsigned pass = 0; pass < 2; ++pass) {
            File file = SD.open(path, FILE_READ);
            if (!file || file.isDirectory()) {
                Log::warn(Log::STORAGE_TAG, "SD diagnostic (%s): cannot open %s", phase, path);
                break;
            }

            const size_t expected = file.size();
            size_t total = 0;
            uint32_t hash = 2166136261u;  // FNV-1a, streaming comparison (no image decoding)
            while (total < expected) {
                const size_t remaining = expected - total;
                const size_t chunk = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
                const size_t count = file.read(buffer, chunk);
                if (count == 0 || count > chunk)
                    break;
                for (size_t i = 0; i < count; ++i)
                    hash = (hash ^ buffer[i]) * 16777619u;
                total += count;
            }
            file.close();

            Log::info(Log::STORAGE_TAG, "SD diagnostic (%s): %s pass=%u bytes=%u/%u fnv1a=%08lx",
                      phase, path, pass + 1, (unsigned)total, (unsigned)expected,
                      (unsigned long)hash);
            if (total != expected) {
                Log::error(Log::STORAGE_TAG, "SD diagnostic: incomplete read of %s", path);
                break;
            }
            if (pass && (firstHash != hash || firstSize != expected))
                Log::error(Log::STORAGE_TAG, "SD diagnostic: inconsistent repeated reads of %s",
                           path);
            firstHash = hash;
            firstSize = expected;
        }
    }
}
#endif

bool StorageSystem::isReady() const {
    return ready;
}

void StorageSystem::listFiles() const {
    if (!ready)
        return;

    SPIBusLock::Guard guard;

    Log::info(Log::STORAGE_TAG, "Path probe: /.core=%s /games=%s /brand.png=%s",
              exists(CORE_DIR) ? "yes" : "no", exists(GAMES_DIR) ? "yes" : "no",
              exists(IMG_BRAND_FILE) ? "yes" : "no");

    std::vector<StorageEntry> coreEntries;

    if (listDirectory(CORE_DIR, coreEntries, true)) {
        Log::info(Log::STORAGE_TAG, "Listing files under /.core:");

        for (const auto& entry : coreEntries) {
            Log::info(Log::STORAGE_TAG, " - %s%s (size: %u bytes)", entry.path.c_str(),
                      entry.isDirectory ? "/" : "", static_cast<uint32_t>(entry.size));
        }
    }

    std::vector<StorageEntry> gameEntries;

    if (listDirectory(GAMES_DIR, gameEntries, true)) {
        Log::info(Log::STORAGE_TAG, "Listing files under /games:");

        for (const auto& entry : gameEntries) {
            Log::info(Log::STORAGE_TAG, " - %s%s (size: %u bytes)", entry.path.c_str(),
                      entry.isDirectory ? "/" : "", static_cast<uint32_t>(entry.size));
        }
    }
}

IFile* StorageSystem::open(const char* path, FileMode mode) {
    if (mode == FileMode::Read) {
        if (auto* cached = GameAssetCache::instance().open(path))
            return cached;
    } else
        GameAssetCache::instance().invalidate(path);
    if (!ready || !driver || !path)
        return nullptr;
    errno = 0;
    IFile* file = driver->open(path, mode);

    if (!file) {
        // Missing optional assets are different from low-level I/O failures.
        if (errno != ENOENT && errno != ENOTDIR)
            noteIOFailure();

        Log::error(Log::STORAGE_TAG, "Failed to open file: %s", path);
#if RECOVERY_STORAGE_SYSTEM_ENABLED
        if (ioFailures >= IO_FAILURE_THRESHOLD) {
            recovery->recover(StorageFailureReason::ConsecutiveIOFailures);
        }
#endif
        return nullptr;
    } else {
        Log::debug(Log::STORAGE_TAG, "Opened file: %s", path);
    }

    return file;
}

StorageSystem::Health StorageSystem::health() const {
    if (!ready)
        return Health::Unavailable;
    const auto failures = ioFailures.load(std::memory_order_relaxed);
    return failures >= IO_FAILURE_THRESHOLD ? Health::Error
           : failures                       ? Health::Degraded
                                            : Health::Ready;
}

void StorageSystem::noteIOFailure() const {
    ioFailures.fetch_add(1, std::memory_order_relaxed);
}

void StorageSystem::noteIOSuccess() const {
    ioFailures.store(0, std::memory_order_relaxed);
}

bool StorageSystem::checkHealth() {
    if (!exists(CORE_DIR)) {
        noteIOFailure();
#if RECOVERY_STORAGE_SYSTEM_ENABLED
        recovery->recover(StorageFailureReason::CorruptionDetected);
#endif
        return false;
    }

    return true;
}

void StorageSystem::update() {
    if (!HEALTH_CHECK_ENABLED)
        return;

    static uint32_t lastCheck = 0;
    if (millis() - lastCheck < HEALTH_CHECK_INTERVAL_MS)
        return;
    lastCheck = millis();
    checkHealth();
}

bool StorageSystem::readTextFile(const char* path, std::string& out, bool required) {
    SPIBusLock::Guard guard;

    IFile* file = open(path);

    if (!file) {
        // Arduino may translate a failed stat after a disk error to ENOENT.
        // A required game file that cannot be read must still degrade its status.
        if (required && (errno == ENOENT || errno == ENOTDIR))
            noteIOFailure();
        return false;
    }

    out.clear();
    const size_t expected = file->length();

    char buffer[256];
    bool complete = true;

    while (out.size() < expected) {
        int r = file->read(reinterpret_cast<uint8_t*>(buffer),
                           std::min(sizeof(buffer), expected - out.size()));

        if (r <= 0) {
            Log::error(Log::STORAGE_TAG, "SD read failed before EOF: %s (%u bytes read)", path,
                       (unsigned)out.size());
            complete = false;
            break;
        }

        out.append(buffer, r);
    }

    const bool physical = !file->isMemoryBacked();
    file->close();
    delete file;

    if (!complete) {
        noteIOFailure();
        out.clear();
    } else if (physical && !out.empty())
        noteIOSuccess();
    return complete;
}

bool StorageSystem::removePath(const char* path) {
    GameAssetCache::instance().invalidate(path);
    if (!ready)
        return false;

    return driver->remove(path);
}

bool StorageSystem::movePath(const char* sourcePath, const char* destinationPath, bool overwrite) {
    GameAssetCache::instance().invalidate(sourcePath);
    GameAssetCache::instance().invalidate(destinationPath);
    SPIBusLock::Guard guard;

    if (!ready)
        return false;

    if (!sourcePath || !destinationPath)
        return false;

    if (!*sourcePath || !*destinationPath)
        return false;

    if (!driver->exists(sourcePath))
        return false;

    if (!driver->ensureParents(destinationPath))
        return false;

    if (overwrite && driver->exists(destinationPath)) {
        if (!driver->remove(destinationPath))
            return false;
    }

    return driver->rename(sourcePath, destinationPath);
}

bool StorageSystem::createDirectory(const char* path) {
    SPIBusLock::Guard guard;

    if (!ready || !path || !*path)
        return false;

    if (driver->exists(path))
        return true;

    if (!driver->ensureParents(path))
        return false;

    return driver->mkdir(path);
}

bool StorageSystem::exists(const char* path) const {
    if (!ready || !path)
        return false;

    return driver->exists(path);
}

bool StorageSystem::listDirectory(const char* path, std::vector<StorageEntry>& out,
                                  bool recursive) const {
    SPIBusLock::Guard guard;

    if (!ready)
        return false;

    std::vector<StorageEntry> localEntries;

#if SD_READ_DIAGNOSTICS_ENABLED
    Log::debug(Log::STORAGE_TAG, "Listing directory: %s (minimum free stack=%u)", path,
               (unsigned)uxTaskGetStackHighWaterMark(nullptr));
#endif
    if (!driver->listDirectory(path, localEntries)) {
        if (errno != ENOENT && errno != ENOTDIR)
            noteIOFailure();
        return false;
    }

    out.insert(out.end(), localEntries.begin(), localEntries.end());

    if (!recursive)
        return true;

    for (const auto& entry : localEntries) {
        if (!entry.isDirectory)
            continue;

        if (entry.name == SYSTEM_VOLUME_INFO_DIR) {
            continue;
        }

        if (!listDirectory(entry.path.c_str(), out, true)) {
            Log::warn(Log::STORAGE_TAG, "Failed to list subdirectory: %s", entry.path.c_str());
        }
    }

    return true;
}

uint64_t StorageSystem::totalBytes() const {
    if (!ready)
        return 0;

    return driver->totalBytes();
}

uint64_t StorageSystem::capacityBytes() const {
    // Cached before publishing readiness: telemetry never waits for the SD bus.
    return ready ? cardCapacity : 0;
}

StorageSystem::SpaceUsage StorageSystem::spaceUsage() const {
    portENTER_CRITICAL(&spaceMutex);
    const auto result = cachedSpace;
    portEXIT_CRITICAL(&spaceMutex);
    return result;
}

void StorageSystem::requestSpaceUsage() {
    portENTER_CRITICAL(&spaceMutex);
    if (cachedSpace.state == SpaceUsage::State::Loading) {
        portEXIT_CRITICAL(&spaceMutex);
        return;
    }
    cachedSpace.state = ready ? SpaceUsage::State::Loading : SpaceUsage::State::Failed;
    portEXIT_CRITICAL(&spaceMutex);
    if (!ready)
        return;
    if (xTaskCreate(spaceUsageTask, "sdSpace", 4096, this, 1, nullptr) != pdPASS) {
        portENTER_CRITICAL(&spaceMutex);
        cachedSpace.state = SpaceUsage::State::Failed;
        portEXIT_CRITICAL(&spaceMutex);
    }
}

void StorageSystem::spaceUsageTask(void* context) {
    auto* storage = static_cast<StorageSystem*>(context);
    SpaceUsage result;
    const bool ok = storage->driver && storage->driver->readSpaceUsage(result.total, result.used);
    result.state = ok ? SpaceUsage::State::Ready : SpaceUsage::State::Failed;
    if (!ok)
        storage->noteIOFailure();
    portENTER_CRITICAL(&storage->spaceMutex);
    storage->cachedSpace = result;
    portEXIT_CRITICAL(&storage->spaceMutex);
    Log::info(Log::STORAGE_TAG, "Space usage: %s, used=%llu total=%llu", ok ? "ready" : "failed",
              (unsigned long long)result.used, (unsigned long long)result.total);
    vTaskDelete(nullptr);
}

uint64_t StorageSystem::usedBytes() const {
    if (!ready)
        return 0;

    return driver->usedBytes();
}

uint64_t StorageSystem::freeBytes() const {
    const uint64_t total = totalBytes();
    const uint64_t used = usedBytes();

    return total > used ? total - used : 0;
}

bool StorageSystem::writeTextFile(const char* path, const std::string& contents, bool overwrite) {
    GameAssetCache::instance().invalidate(path);
    SPIBusLock::Guard guard;

    if (!ready || !path)
        return false;

    if (!driver->ensureParents(path))
        return false;

    if (overwrite && driver->exists(path)) {
        if (!driver->remove(path))
            return false;
    }

    IFile* file = open(path, FileMode::Write);

    if (!file) {
        Log::error(Log::STORAGE_TAG, "Failed to open file for write: %s", path);

        return false;
    }

    size_t written = 0;
    const auto* data = reinterpret_cast<const uint8_t*>(contents.data());
    while (written < contents.size()) {
        const size_t chunk = file->write(data + written, contents.size() - written);
        if (chunk == 0)
            break;
        written += chunk;
    }

    file->flush();
    file->close();
    delete file;

    const bool complete = written == contents.size();
    if (!complete)
        noteIOFailure();
    return complete;
}

File StorageSystem::openNative(const char* path, bool write) {
    if (write)
        GameAssetCache::instance().invalidate(path);
    if (!ready || !driver || !path)
        return File();
    errno = 0;
    File file = static_cast<ESP32SDCardDriver*>(driver)->openNative(path, write);
    if (!file && errno != ENOENT && errno != ENOTDIR)
        noteIOFailure();
    return file;
}

uint64_t StorageSystem::directorySize(const char* path) const {
    SPIBusLock::Guard guard;

    std::vector<StorageEntry> entries;

    if (!listDirectory(path, entries, true))
        return 0;

    uint64_t total = 0;

    for (const auto& entry : entries) {
        if (!entry.isDirectory)
            total += entry.size;
    }

    return total;
}

bool StorageSystem::cleanupDirectoryBySize(const char* path, uint64_t maxSize,
                                           uint64_t targetSize) {
    SPIBusLock::Guard guard;

    if (!ready)
        return false;

    std::vector<StorageEntry> entries;
    if (!listDirectory(path, entries, false))
        return false;

    uint64_t total = 0;
    for (const auto& e : entries) {
        if (!e.isDirectory)
            total += e.size;
    }

    if (total <= maxSize)
        return true;

    std::sort(entries.begin(), entries.end(), [](const StorageEntry& a, const StorageEntry& b) {
        return a.name < b.name;
    });

    for (const auto& entry : entries) {
        if (entry.isDirectory)
            continue;

        if (total <= targetSize)
            break;

        Log::info(Log::STORAGE_TAG, "Removing %s", entry.path.c_str());
        if (removePath(entry.path.c_str())) {
            total -= entry.size;
        }
    }

    return true;
}
