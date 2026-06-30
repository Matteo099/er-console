#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD.h>

#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "drivers/storage/IFile.h"


static constexpr uint64_t MAX_LOG_FOLDER_SIZE = 10ULL * 1024 * 1024; // 10 MB
static constexpr uint64_t TARGET_AFTER_CLEANUP = 9ULL * 1024 * 1024; // hysteresis

class StorageSystem;

class LogStorageWriter
{
public:
    static LogStorageWriter &instance();

    void start(StorageSystem *storage);

    void write(const char *line);

private:
    LogStorageWriter() = default;

    static void task(void *param);

    void run();


private:
    QueueHandle_t queue = nullptr;

    StorageSystem *storageSystem = nullptr;

    IFile *logFile = nullptr;

    bool initialized = false;
};