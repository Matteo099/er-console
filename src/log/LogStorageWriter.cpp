#include "LogStorageWriter.h"

#include <time.h>

#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"

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

    // non blocking
    if (xQueueSend(queue, &copy, 0) != pdTRUE)
    {
        free(copy);
    }
}

void LogStorageWriter::task(void *param)
{
    auto *self =
        static_cast<LogStorageWriter *>(param);

    self->run();
}

void LogStorageWriter::run()
{
    while (true)
    {
        char *line = nullptr;

        if (xQueueReceive(
                queue,
                &line,
                portMAX_DELAY) == pdTRUE)
        {

            if (!logFile)
            {
                if (!SD.exists(LOGS_DIR))
                    SD.mkdir(LOGS_DIR);

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

                logFile = SD.open(
                    filename,
                    FILE_APPEND);
            }

            if (logFile)
            {
                logFile.println(line);

                // optional batching
                logFile.flush();
            }

            free(line);
        }
    }
}