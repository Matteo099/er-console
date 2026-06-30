#include "LogStorageWriter.h"

#include <time.h>

#include "config/Assets.h"
#include "systems/storage/StorageSystem.h"

LogStorageWriter &LogStorageWriter::instance()
{
    static LogStorageWriter writer;
    return writer;
}

void LogStorageWriter::start(StorageSystem *storage)
{
    storageSystem = storage;

    if (!storageSystem)
        return;

    queue = xQueueCreate(
        100,
        sizeof(char *));

    xTaskCreatePinnedToCore(
        task,
        "LogWriter",
        4096,
        this,
        1,
        nullptr,
        1);

    initialized = true;
}

void LogStorageWriter::write(const char *line)
{
    if (!initialized || !queue)
        return;

    char *copy = strdup(line);

    if (!copy)
        return;

    if (xQueueSend(queue, &copy, 0) != pdTRUE)
    {
        free(copy);
    }
}

void LogStorageWriter::task(void *param)
{
    static_cast<LogStorageWriter *>(param)->run();
}

void LogStorageWriter::run()
{
    while (true)
    {
        char *line = nullptr;

        if (xQueueReceive(
                queue,
                &line,
                portMAX_DELAY) != pdTRUE)
        {
            continue;
        }

        if (logFile == nullptr && storageSystem != nullptr)
        {
            storageSystem->createDirectory(LOGS_DIR);

            storageSystem->cleanupDirectoryBySize(
                LOGS_DIR,
                MAX_LOG_FOLDER_SIZE,
                TARGET_AFTER_CLEANUP);

            char filename[64];

            struct tm timeinfo;

            if (getLocalTime(&timeinfo))
            {
                strftime(
                    filename,
                    sizeof(filename),
                    LOGS_DIR "/%Y-%m-%d_%H-%M-%S.log",
                    &timeinfo);
            }
            else
            {
                strcpy(
                    filename,
                    LOGS_DIR "/boot.log");
            }

            logFile = storageSystem->open(filename, FileMode::Append);
        }

        if (logFile != nullptr)
        {
            logFile->writeLine(line, true);
        }

        free(line);
    }
}