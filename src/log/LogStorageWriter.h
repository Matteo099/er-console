#pragma once

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

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

    File logFile;

    bool initialized = false;
};