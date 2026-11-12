#pragma once

#include <SD.h>
#include <vector>
#include <string>
#include <atomic>
#include "core/SystemService.h"
#include "drivers/storage/SDCardDriver.h"
#include "drivers/storage/IFile.h"
#include "drivers/storage/StorageEntry.h"
#include "drivers/storage/StorageRecoverySystem.h"
#include "config/Console.h"

#define IO_FAILURE_THRESHOLD 5
#define HEALTH_CHECK_INTERVAL_MS 30000
#define HEALTH_CHECK_ENABLED 1

class StorageSystem : public SystemService {
  public:
    enum class Health { Unavailable, Ready, Degraded, Error };

    struct SpaceUsage {
        enum class State { Unknown, Loading, Ready, Failed };
        State state = State::Unknown;
        uint64_t total = 0, used = 0;
    };

    StorageSystem();

    void init(void* params = nullptr) override;
    bool isReady() const;
    Health health() const;
    void noteIOFailure() const;
    void noteIOSuccess() const;
    void update() override;

    virtual IFile* open(const char* path, FileMode mode = FileMode::Read);
    bool readTextFile(const char* path, std::string& out, bool required = false);
    bool writeTextFile(const char* path, const std::string& contents, bool overwrite = true);
    File openNative(const char* path, bool write = false);
    bool removePath(const char* path);
    bool movePath(const char* sourcePath, const char* destinationPath, bool overwrite = true);
    bool createDirectory(const char* path);
    bool exists(const char* path) const;
    bool listDirectory(const char* path, std::vector<StorageEntry>& out,
                       bool recursive = false) const;
    uint64_t totalBytes() const;
    uint64_t capacityBytes() const;
    uint64_t usedBytes() const;
    uint64_t freeBytes() const;
    void requestSpaceUsage();
    SpaceUsage spaceUsage() const;
    void listFiles() const;
#if SD_READ_DIAGNOSTICS_ENABLED
    void runReadDiagnostics(const char* phase) const;
#endif
    bool createDirectoryIfMissing(const char* path);
    uint64_t directorySize(const char* path) const;
    bool cleanupDirectoryBySize(const char* path, uint64_t maxSize, uint64_t targetSize);

  private:
    bool checkHealth();
    static void spaceUsageTask(void* context);
    mutable portMUX_TYPE spaceMutex = portMUX_INITIALIZER_UNLOCKED;
    SpaceUsage cachedSpace;

  private:
    SDCardDriver* driver = nullptr;
#if RECOVERY_STORAGE_SYSTEM_ENABLED
    StorageRecoverySystem* recovery;
#endif
    bool ready = false;
    uint64_t cardCapacity = 0;
    mutable std::atomic<uint32_t> ioFailures{0};
};
