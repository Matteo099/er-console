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

    if (storageSystem == nullptr)
        return;

    queue = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(LogMessage));

    if (queue == nullptr)
        return;

    xTaskCreatePinnedToCore(
        task,
        "LogWriter",
        6144,
        this,
        1,
        nullptr,
        1);

    initialized = true;
}

void LogStorageWriter::write(const char *line)
{
    if (!initialized || queue == nullptr || line == nullptr)
        return;

    LogMessage msg{};

    strncpy(
        msg.text,
        line,
        sizeof(msg.text) - 1);

    msg.text[sizeof(msg.text) - 1] = '\0';

    xQueueSend(queue, &msg, 0);
}

void LogStorageWriter::task(void *param)
{
    static_cast<LogStorageWriter *>(param)->run();
}

bool LogStorageWriter::ensureLogFile()
{
    if (logFile != nullptr)
        return true;

    if (storageSystem == nullptr)
        return false;

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

    logFile = storageSystem->open(
        filename,
        FileMode::Append);

    return logFile != nullptr;
}

void LogStorageWriter::flushBuffer()
{
    if (writeBufferUsed == 0)
        return;

    if (!ensureLogFile())
        return;

    logFile->write(
        reinterpret_cast<const uint8_t *>(writeBuffer),
        writeBufferUsed);

    logFile->flush();

    writeBufferUsed = 0;
}

void LogStorageWriter::run()
{
    TickType_t lastFlush = xTaskGetTickCount();

    while (true)
    {
        LogMessage msg;

        bool received =
            xQueueReceive(
                queue,
                &msg,
                pdMS_TO_TICKS(100)) == pdTRUE;

        if (received)
        {
            do
            {
                size_t len = strlen(msg.text);

                if (len >= sizeof(msg.text))
                    len = sizeof(msg.text) - 1;

                if (writeBufferUsed + len + 1 > BUFFER_SIZE)
                {
                    flushBuffer();
                    lastFlush = xTaskGetTickCount();
                }

                memcpy(
                    writeBuffer + writeBufferUsed,
                    msg.text,
                    len);

                writeBufferUsed += len;

                writeBuffer[writeBufferUsed++] = '\n';

            } while (
                xQueueReceive(
                    queue,
                    &msg,
                    0) == pdTRUE);
        }

        TickType_t now = xTaskGetTickCount();

        if (writeBufferUsed > 0 &&
            (now - lastFlush) >= FLUSH_INTERVAL)
        {
            flushBuffer();
            lastFlush = now;
        }
    }
}