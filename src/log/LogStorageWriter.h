#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "drivers/storage/IFile.h"

static constexpr uint64_t MAX_LOG_FOLDER_SIZE = 10ULL * 1024 * 1024;
static constexpr uint64_t TARGET_AFTER_CLEANUP = 9ULL * 1024 * 1024;

class StorageSystem;

class LogStorageWriter {
  public:
    static LogStorageWriter& instance();

    void start(StorageSystem* storage);

    void write(const char* line);

  private:
    LogStorageWriter() = default;

    static void task(void* param);

    void run();

    bool ensureLogFile();

    void flushBuffer();

  private:
    struct LogMessage {
        char text[256];
    };

    static constexpr size_t QUEUE_LENGTH = 100;

    static constexpr size_t BUFFER_SIZE = 8192;

    static constexpr TickType_t FLUSH_INTERVAL = pdMS_TO_TICKS(2000);

    QueueHandle_t queue = nullptr;

    StorageSystem* storageSystem = nullptr;

    IFile* logFile = nullptr;

    bool initialized = false;

    char writeBuffer[BUFFER_SIZE];

    size_t writeBufferUsed = 0;
};