/*
Dopo l’avvio puoi inviare questi caratteri dal monitor seriale:

1 → FULL
2 → ENDURANCE
3 → RANDOM READ
4 → FILESYSTEM
5 → DISPLAY ONLY
P → pausa test SD
R → ritorna a FULL

Modalità disponibili:
- FULL: lettura, modifica, scrittura e verifica del file da 100 KB;
- ENDURANCE: stesso test con pausa ridotta a 50 ms;
- RANDOM READ: 128 seek/letture casuali per ciclo;
- FILESYSTEM: crea, scrive, legge, verifica e cancella 8 file da 8 KB;
- DISPLAY ONLY: sospende la SD e mantiene il refresh TFT/touch;
- PAUSED: sospende le operazioni SD.
*/



#include <Arduino.h>
#include <SPI.h>

#include "config/BoardConfig.h"
#include "log/Logger.h"

// ------------------------------------------------------------
// Storage
// ------------------------------------------------------------

#include "drivers/storage/ESP32SDCardDriver.h"

// ------------------------------------------------------------
// Display / LVGL
// ------------------------------------------------------------

#include "drivers/display/TFTDisplay.h"
#include "systems/ui/lvgl/LVGLContext.h"

// ------------------------------------------------------------
// SPI synchronization
// ------------------------------------------------------------

#include "drivers/bus/SPIBusLock.h"

// ============================================================
// GLOBAL OBJECTS
// ============================================================

// The SD card has its own SPI host on the ESP32-S3.  Use the bus manager's
// instance so that every SD-side user shares the same peripheral and lock.
SPIClass &sdSpi = SPIBusManager::bus();

ESP32SDCardDriver sd(sdSpi, SD_CS_PIN);

static TFTDisplay display;
static LVGLContext lvgl;

// ============================================================
// TEST CONFIGURATION
// ============================================================

static constexpr const char *TEST_FILE = "/tft_sd_test.txt";
static constexpr const char *SD_STRESS_FILE = "/sd_stress.bin";
static constexpr size_t SD_STRESS_SIZE = 100 * 1024;
static constexpr size_t SD_STRESS_BLOCK = 4096;
static constexpr size_t FS_STRESS_FILE_SIZE = 8 * 1024;
static constexpr uint8_t FS_STRESS_FILE_COUNT = 8;

enum class StressMode : uint8_t
{
    FullCombined,
    Endurance,
    RandomRead,
    Filesystem,
    DisplayOnly,
    Paused
};

static volatile StressMode stressMode = StressMode::FullCombined;

static constexpr uint32_t TEST_INTERVAL_MS = 5000;

// ============================================================
// TEST STATE
// ============================================================

static volatile uint32_t sdWriteSuccess = 0;
static volatile uint32_t sdWriteFail = 0;

static volatile uint32_t sdReadSuccess = 0;
static volatile uint32_t sdReadFail = 0;

static volatile uint32_t buttonClicks = 0;

static volatile bool sdReady = false;

static volatile bool lastWriteOK = false;
static volatile bool lastReadOK = false;

static char lastReadLine[128] = "Waiting for SD read...";

static SemaphoreHandle_t stateMutex = nullptr;

// ============================================================
// LVGL OBJECTS
// ============================================================

static lv_obj_t *writeStatusLabel = nullptr;
static lv_obj_t *readStatusLabel = nullptr;
static lv_obj_t *lastLineLabel = nullptr;
static lv_obj_t *statisticsLabel = nullptr;
static lv_obj_t *buttonStatusLabel = nullptr;
static lv_obj_t *displayLoadBar = nullptr;
static uint32_t displayFrames = 0;

// ============================================================
// HELPERS
// ============================================================

static void updateLastLine(const char *line)
{
    if (!stateMutex)
        return;

    if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        strncpy(lastReadLine, line, sizeof(lastReadLine) - 1);
        lastReadLine[sizeof(lastReadLine) - 1] = '\0';
        xSemaphoreGive(stateMutex);
    }
}

static const char *stressModeName(StressMode mode)
{
    switch (mode)
    {
    case StressMode::FullCombined:
        return "FULL";
    case StressMode::Endurance:
        return "ENDURANCE";
    case StressMode::RandomRead:
        return "RANDOM READ";
    case StressMode::Filesystem:
        return "FILESYSTEM";
    case StressMode::DisplayOnly:
        return "DISPLAY";
    case StressMode::Paused:
        return "PAUSED";
    }
    return "UNKNOWN";
}

static void selectStressMode(StressMode mode)
{
    stressMode = mode;
    char status[64];
    snprintf(status, sizeof(status), "MODE: %s", stressModeName(mode));
    updateLastLine(status);
    Log::info(Log::DRIVER_TAG, "[STRESS] mode=%s", stressModeName(mode));
}

static void processStressCommands()
{
    while (Serial.available() > 0)
    {
        char command = static_cast<char>(Serial.read());
        switch (command)
        {
        case '1':
            selectStressMode(StressMode::FullCombined);
            break;
        case '2':
            selectStressMode(StressMode::Endurance);
            break;
        case '3':
            selectStressMode(StressMode::RandomRead);
            break;
        case '4':
            selectStressMode(StressMode::Filesystem);
            break;
        case '5':
            selectStressMode(StressMode::DisplayOnly);
            break;
        case 'p':
        case 'P':
            selectStressMode(StressMode::Paused);
            break;
        case 'r':
        case 'R':
            selectStressMode(StressMode::FullCombined);
            break;
        default:
            break;
        }
    }
}

static bool writeExactFile(const char *path, const uint8_t *data, size_t length)
{
    if (!sd.remove(path) && sd.exists(path))
        return false;

    IFile *file = sd.open(path, FileMode::Write);
    if (!file)
        return false;

    size_t written = 0;
    while (written < length)
    {
        size_t n = file->write(data + written, length - written);
        if (n == 0)
            break;
        written += n;
    }

    file->flush();
    file->close();
    delete file;
    return written == length;
}

static bool readExactFile(const char *path, uint8_t *data, size_t length)
{
    IFile *file = sd.open(path, FileMode::Read);
    if (!file)
        return false;

    size_t read = 0;
    while (read < length)
    {
        int n = file->read(data + read, length - read);
        if (n <= 0)
            break;
        read += static_cast<size_t>(n);
    }

    file->close();
    delete file;
    return read == length;
}

static bool runRandomReadCycle(uint32_t cycle, uint8_t *verifyBlock)
{
    IFile *file = sd.open(SD_STRESS_FILE, FileMode::Read);
    if (!file)
        return false;

    uint32_t random = cycle * 1664525UL + 1013904223UL;
    bool ok = true;
    for (uint16_t i = 0; i < 128 && ok; ++i)
    {
        random = random * 1664525UL + 1013904223UL;
        size_t offset = random % (SD_STRESS_SIZE - 512);
        if (!file->seek(offset))
        {
            ok = false;
            break;
        }

        size_t received = 0;
        while (received < 512)
        {
            int n = file->read(verifyBlock + received, 512 - received);
            if (n <= 0)
                break;
            received += static_cast<size_t>(n);
        }
        if (received != 512)
            ok = false;
    }

    file->close();
    delete file;
    return ok;
}

// ============================================================
// SD STRESS TASK
// ============================================================

static void sdStressTask(void *parameter)
{
    LV_UNUSED(parameter);

    uint8_t *buffer = static_cast<uint8_t *>(malloc(SD_STRESS_SIZE));
    uint8_t *verifyBlock = static_cast<uint8_t *>(malloc(SD_STRESS_BLOCK));
    uint8_t *fsBuffer = static_cast<uint8_t *>(malloc(FS_STRESS_FILE_SIZE));

    if (!buffer || !verifyBlock || !fsBuffer)
    {
        free(buffer);
        free(verifyBlock);
        free(fsBuffer);
        Log::error(Log::DRIVER_TAG, "[SD STRESS] RAM allocation failed");
        sdReady = false;
        vTaskDelete(nullptr);
        return;
    }

    Log::info(Log::DRIVER_TAG, "[SD STRESS] STARTED: %u KB file, %u KB blocks",
              (unsigned)(SD_STRESS_SIZE / 1024),
              (unsigned)(SD_STRESS_BLOCK / 1024));

    uint32_t cycle = 0;

    while (true)
    {
        StressMode mode = stressMode;
        if (!sdReady || mode == StressMode::DisplayOnly || mode == StressMode::Paused)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ++cycle;

        if (mode == StressMode::RandomRead)
        {
            bool ok;
            {
                SPIBusLock::Guard guard;
                ok = runRandomReadCycle(cycle, verifyBlock);
            }

            if (ok)
            {
                ++sdReadSuccess;
                Log::info(Log::DRIVER_TAG, "[SD RANDOM] cycle=%lu OK (128 seeks)", (unsigned long)cycle);
            }
            else
            {
                ++sdReadFail;
                updateLastLine("RANDOM READ FAILED");
                Log::error(Log::DRIVER_TAG, "[SD RANDOM] cycle=%lu FAILED", (unsigned long)cycle);
            }
            vTaskDelay(pdMS_TO_TICKS(250));
            continue;
        }

        if (mode == StressMode::Filesystem)
        {
            bool ok = true;
            {
                SPIBusLock::Guard guard;
                for (uint8_t fileIndex = 0; fileIndex < FS_STRESS_FILE_COUNT && ok; ++fileIndex)
                {
                    char path[32];
                    snprintf(path, sizeof(path), "/fs_stress_%02u.bin", (unsigned)fileIndex);
                    uint8_t pattern = static_cast<uint8_t>(cycle + fileIndex * 17);

                    memset(fsBuffer, pattern, FS_STRESS_FILE_SIZE);
                    if (!writeExactFile(path, fsBuffer, FS_STRESS_FILE_SIZE))
                    {
                        ok = false;
                        break;
                    }

                    memset(fsBuffer, 0, FS_STRESS_FILE_SIZE);
                    if (!readExactFile(path, fsBuffer, FS_STRESS_FILE_SIZE))
                    {
                        ok = false;
                        break;
                    }

                    for (size_t i = 0; i < FS_STRESS_FILE_SIZE; ++i)
                    {
                        if (fsBuffer[i] != pattern)
                        {
                            ok = false;
                            break;
                        }
                    }
                    sd.remove(path);
                }
            }

            if (ok)
            {
                ++sdReadSuccess;
                ++sdWriteSuccess;
                Log::info(Log::DRIVER_TAG, "[SD FS] cycle=%lu OK (%u files)",
                          (unsigned long)cycle, (unsigned)FS_STRESS_FILE_COUNT);
            }
            else
            {
                ++sdReadFail;
                ++sdWriteFail;
                updateLastLine("FILESYSTEM TEST FAILED");
                Log::error(Log::DRIVER_TAG, "[SD FS] cycle=%lu FAILED", (unsigned long)cycle);
            }
            vTaskDelay(pdMS_TO_TICKS(250));
            continue;
        }

        bool readOk = true;
        bool writeOk = true;
        size_t totalRead = 0;

        // Keep the complete read/modify/write/verify sequence atomic from
        // the filesystem point of view. The recursive bus mutex also covers
        // the locks taken internally by ESP32SDCardDriver and ESP32SDFile.
        {
            SPIBusLock::Guard guard;

            IFile *file = sd.open(SD_STRESS_FILE, FileMode::Read);
            if (!file)
            {
                // First cycle: create a known deterministic 100 KB file.
                memset(buffer, 0xA5, SD_STRESS_SIZE);
            }
            else
            {
                while (totalRead < SD_STRESS_SIZE)
                {
                    size_t requested = min(SD_STRESS_BLOCK, SD_STRESS_SIZE - totalRead);
                    int n = file->read(buffer + totalRead, requested);
                    if (n <= 0)
                        break;
                    totalRead += static_cast<size_t>(n);
                }
                file->close();
                delete file;
                readOk = (totalRead == SD_STRESS_SIZE);
            }

            if (!readOk)
            {
                ++sdReadFail;
                updateLastLine("STRESS READ FAILED");
                Log::error(Log::DRIVER_TAG, "[SD STRESS] READ FAILED cycle=%lu bytes=%u",
                           (unsigned long)cycle, (unsigned)totalRead);
            }
            else
            {
                ++sdReadSuccess;
            }

            // Modify several locations in the 100 KB image.
            for (size_t i = 0; i < 512; i += 16)
            {
                size_t index = (static_cast<size_t>(cycle) * 7919 + i * 37) % SD_STRESS_SIZE;
                buffer[index] ^= static_cast<uint8_t>(cycle + i);
            }

            // Rewrite the complete file in 4 KB chunks.
            sd.remove(SD_STRESS_FILE);
            file = sd.open(SD_STRESS_FILE, FileMode::Write);
            if (!file)
            {
                writeOk = false;
            }
            else
            {
                size_t totalWritten = 0;
                while (totalWritten < SD_STRESS_SIZE)
                {
                    size_t requested = min(SD_STRESS_BLOCK, SD_STRESS_SIZE - totalWritten);
                    size_t n = file->write(buffer + totalWritten, requested);
                    if (n == 0)
                        break;
                    totalWritten += n;
                }
                file->flush();
                file->close();
                delete file;
                writeOk = (totalWritten == SD_STRESS_SIZE);

                if (!writeOk)
                {
                    Log::error(Log::DRIVER_TAG, "[SD STRESS] WRITE SHORT cycle=%lu bytes=%u/%u",
                               (unsigned long)cycle,
                               (unsigned)totalWritten,
                               (unsigned)SD_STRESS_SIZE);
                }
            }

            if (!writeOk)
            {
                ++sdWriteFail;
                updateLastLine("STRESS WRITE FAILED");
                Log::error(Log::DRIVER_TAG, "[SD STRESS] WRITE FAILED cycle=%lu",
                           (unsigned long)cycle);
            }
            // Read the saved file again and compare every block.
            if (writeOk)
            {
                file = sd.open(SD_STRESS_FILE, FileMode::Read);
                size_t verified = 0;
                while (file && verified < SD_STRESS_SIZE)
                {
                    size_t requested = min(SD_STRESS_BLOCK, SD_STRESS_SIZE - verified);
                    size_t blockRead = 0;
                    while (blockRead < requested)
                    {
                        int n = file->read(verifyBlock + blockRead, requested - blockRead);
                        if (n <= 0)
                            break;
                        blockRead += static_cast<size_t>(n);
                    }

                    if (blockRead != requested ||
                        memcmp(verifyBlock, buffer + verified, requested) != 0)
                    {
                        writeOk = false;
                        break;
                    }
                    verified += requested;
                }
                if (file)
                {
                    file->close();
                    delete file;
                }

                if (!writeOk)
                {
                    ++sdWriteFail;
                    updateLastLine("STRESS VERIFY FAILED");
                    Log::error(Log::DRIVER_TAG, "[SD STRESS] VERIFY FAILED cycle=%lu",
                               (unsigned long)cycle);
                }
            }

            if (readOk && writeOk)
            {
                ++sdWriteSuccess;
                char status[128];
                snprintf(status, sizeof(status), "STRESS cycle %lu OK - 100 KB", (unsigned long)cycle);
                updateLastLine(status);
                Log::info(Log::DRIVER_TAG, "[SD STRESS] cycle=%lu OK", (unsigned long)cycle);
            }
        }

        lastReadOK = readOk;
        lastWriteOK = writeOk;

        // Short pause: enough to let the TFT task render and the touch
        // controller be sampled between two SD stress cycles.
        vTaskDelay(pdMS_TO_TICKS(mode == StressMode::Endurance ? 50 : 250));
    }
}

// ============================================================
// SD WRITE TASK
// ============================================================

static void sdWriteTask(void *parameter)
{
    LV_UNUSED(parameter);
    Log::info(Log::DRIVER_TAG, "========================================");
    Log::info(Log::DRIVER_TAG, "SD WRITE TASK STARTED");
    Log::info(Log::DRIVER_TAG, "========================================");
    uint32_t counter = 0;

    while (true)
    {
        if (!sdReady)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        ++counter;

        // ----------------------------------------------------
        // Build line
        // ----------------------------------------------------

        char line[128];
        snprintf(line, sizeof(line), "SD TEST LINE #%lu - hello from ESP32", (unsigned long)counter);
        Log::info(Log::DRIVER_TAG, "[SD WRITE] Appending: %s", line);

        // ----------------------------------------------------
        // Open file
        // ----------------------------------------------------

        /*
         * IMPORTANT:
         *
         * We use the driver rather than SD.open().
         *
         * FileMode::Append preserves the previous test lines.
         */

        {
            SPIBusLock::Guard guard;
            IFile *file = sd.open(TEST_FILE, FileMode::Append);
            if (!file)
            {
                ++sdWriteFail;
                lastWriteOK = false;
                Log::error(Log::DRIVER_TAG, "[SD WRITE] OPEN FAILED: %s", TEST_FILE);
                continue;
            }

        // ----------------------------------------------------
        // Append line
        // ----------------------------------------------------

            char lineWithNewline[132];
            snprintf(lineWithNewline, sizeof(lineWithNewline), "%s\n", line);
            const size_t length = strlen(lineWithNewline);
            size_t written = file->write(reinterpret_cast<const uint8_t *>(lineWithNewline), length);
            if (written != length)
            {
                ++sdWriteFail;
                lastWriteOK = false;
                Log::error(Log::DRIVER_TAG, "[SD WRITE] WRITE FAILED expected=%u written=%u", (unsigned)length, (unsigned)written);
                delete file;
                continue;
            }

        // ----------------------------------------------------
        // Flush / close
        // ----------------------------------------------------

            file->flush();
            delete file;
            ++sdWriteSuccess;
            lastWriteOK = true;
            Log::info(Log::DRIVER_TAG, "[SD WRITE] SUCCESS #%lu", (unsigned long)sdWriteSuccess);
        }

        // ----------------------------------------------------
        // Wait
        // ----------------------------------------------------

        vTaskDelay(pdMS_TO_TICKS(TEST_INTERVAL_MS));
    }
}

// ============================================================
// SD READ TASK
// ============================================================

static void sdReadTask(void *parameter)
{
    LV_UNUSED(parameter);

    Log::info(Log::DRIVER_TAG, "========================================");
    Log::info(Log::DRIVER_TAG, "SD READ TASK STARTED");
    Log::info(Log::DRIVER_TAG, "========================================");

    while (true)
    {
        if (!sdReady)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        Log::info(Log::DRIVER_TAG, "[SD READ] Reading last line");

        // ----------------------------------------------------
        // Open file
        // ----------------------------------------------------

        {
            SPIBusLock::Guard guard;
            IFile *file = sd.open(TEST_FILE, FileMode::Read);
            if (!file)
            {
                ++sdReadFail;
                lastReadOK = false;
                updateLastLine("READ FAILED: open");
                Log::error(Log::DRIVER_TAG, "[SD READ] OPEN FAILED");
                continue;
            }

        // ----------------------------------------------------
        // Read complete file
        //
        // For this test the file is intentionally small.
        // ----------------------------------------------------

            char buffer[1024];

            memset(buffer, 0, sizeof(buffer));

            size_t totalRead = 0;

            while (totalRead < sizeof(buffer) - 1)
            {
                size_t n = file->read(reinterpret_cast<uint8_t *>(buffer + totalRead), sizeof(buffer) - totalRead - 1);
                if (n == 0)
                    break;
                totalRead += n;
            }

            delete file;

            if (totalRead == 0)
            {
                ++sdReadFail;
                lastReadOK = false;
                updateLastLine("READ FAILED: empty file");
                Log::error(Log::DRIVER_TAG, "[SD READ] Empty file");
                continue;
            }

            buffer[totalRead] = '\0';

        // ----------------------------------------------------
        // Find last line
        // ----------------------------------------------------
            char *lastLine = buffer;
            for (size_t i = 0; i < totalRead; ++i)
            {
                if (buffer[i] == '\n')
                {
                    if (i + 1 < totalRead)
                    {
                        lastLine = &buffer[i + 1];
                    }
                }
            }

        // ----------------------------------------------------
        // Remove trailing newline
        // ----------------------------------------------------
            size_t lastLength = strlen(lastLine);
            while (lastLength > 0 && (lastLine[lastLength - 1] == '\n' || lastLine[lastLength - 1] == '\r'))
            {
                lastLine[lastLength - 1] = '\0';
                --lastLength;
            }
            if (strlen(lastLine) == 0)
            {
                ++sdReadFail;
                lastReadOK = false;
                updateLastLine("READ FAILED: invalid line");
                Log::error(Log::DRIVER_TAG, "[SD READ] Last line empty");
                continue;
            }

        // ----------------------------------------------------
        // Success
        // ----------------------------------------------------
            ++sdReadSuccess;
            lastReadOK = true;
            updateLastLine(lastLine);
            Log::info(Log::DRIVER_TAG, "[SD READ] SUCCESS: '%s'", lastLine);
        }

        // ----------------------------------------------------
        // Wait
        // ----------------------------------------------------

        vTaskDelay(pdMS_TO_TICKS(TEST_INTERVAL_MS));
    }
}

// ============================================================
// BUTTON CALLBACK
// ============================================================

static void testButtonCallback(lv_event_t *event)
{
    LV_UNUSED(event);
    ++buttonClicks;
    Log::info(Log::DISPLAY_TAG, "[TFT] TEST BUTTON CLICKED #%lu", (unsigned long)buttonClicks);
    if (buttonStatusLabel)
    {
        char text[64];
        snprintf(text, sizeof(text), "Button clicks: %lu", (unsigned long)buttonClicks);
        lv_label_set_text(buttonStatusLabel, text);
    }
}

// ============================================================
// CREATE TFT UI
// ============================================================

static void createTFTUI()
{
    Log::info(Log::DISPLAY_TAG, "Creating TFT test UI");
    lv_obj_t *screen = lv_screen_active();

    // --------------------------------------------------------
    // Background
    // --------------------------------------------------------
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101010), LV_PART_MAIN);

    // --------------------------------------------------------
    // Title
    // --------------------------------------------------------
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "SD + TFT STRESS");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    // --------------------------------------------------------
    // SD WRITE STATUS
    // --------------------------------------------------------
    writeStatusLabel = lv_label_create(screen);
    lv_label_set_text(writeStatusLabel, "WRITE: waiting...");
    lv_obj_set_style_text_color(writeStatusLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(writeStatusLabel, LV_ALIGN_TOP_LEFT, 10, 50);

    // --------------------------------------------------------
    // SD READ STATUS
    // --------------------------------------------------------
    readStatusLabel = lv_label_create(screen);
    lv_label_set_text(readStatusLabel, "READ: waiting...");
    lv_obj_set_style_text_color(readStatusLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(readStatusLabel, LV_ALIGN_TOP_LEFT, 10, 75);

    // --------------------------------------------------------
    // LAST LINE
    // --------------------------------------------------------
    lv_obj_t *lastLineTitle = lv_label_create(screen);
    lv_label_set_text(lastLineTitle, "Last SD line:");
    lv_obj_align(lastLineTitle, LV_ALIGN_TOP_LEFT, 10, 105);
    lastLineLabel = lv_label_create(screen);
    lv_label_set_text(lastLineLabel, "Waiting...");
    lv_label_set_long_mode(lastLineLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lastLineLabel, 300);
    lv_obj_align(lastLineLabel, LV_ALIGN_TOP_LEFT, 10, 125);

    // --------------------------------------------------------
    // STATISTICS
    // --------------------------------------------------------

    statisticsLabel = lv_label_create(screen);
    lv_label_set_text(statisticsLabel, "WRITE  OK: 0  FAIL: 0\n"
                                       "READ   OK: 0  FAIL: 0");
    lv_obj_align(statisticsLabel, LV_ALIGN_TOP_LEFT, 10, 160);

    displayLoadBar = lv_bar_create(screen);
    lv_obj_set_size(displayLoadBar, 210, 16);
    lv_obj_align(displayLoadBar, LV_ALIGN_TOP_LEFT, 10, 205);
    lv_bar_set_range(displayLoadBar, 0, 100);

    lv_obj_t *displayLoadLabel = lv_label_create(screen);
    lv_label_set_text(displayLoadLabel, "TFT continuous refresh");
    lv_obj_align(displayLoadLabel, LV_ALIGN_TOP_LEFT, 10, 225);

    // --------------------------------------------------------
    // TEST BUTTON
    // --------------------------------------------------------

    lv_obj_t *button = lv_button_create(screen);
    lv_obj_set_size(button, 150, 50);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -35);
    lv_obj_add_event_cb(button, testButtonCallback, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *buttonLabel = lv_label_create(button);
    lv_label_set_text(buttonLabel, "TEST BUTTON");
    lv_obj_center(buttonLabel);

    // --------------------------------------------------------
    // BUTTON STATUS
    // --------------------------------------------------------

    buttonStatusLabel = lv_label_create(screen);
    lv_label_set_text(buttonStatusLabel, "Button clicks: 0");
    lv_obj_align(buttonStatusLabel, LV_ALIGN_BOTTOM_MID, 0, -8);
    Log::info(Log::DISPLAY_TAG, "TFT test UI created");
}

// ============================================================
// TFT / LVGL TASK
// ============================================================

static void tftTask(void *parameter)
{
    LV_UNUSED(parameter);

    Log::info(Log::DISPLAY_TAG, "========================================");
    Log::info(Log::DISPLAY_TAG, "TFT TASK STARTED");
    Log::info(Log::DISPLAY_TAG, "========================================");

    while (true)
    {
        processStressCommands();

        // ----------------------------------------------------
        // Update LVGL
        // ----------------------------------------------------

        lvgl.update();

        // ----------------------------------------------------
        // Update SD status on screen
        //
        // LVGL is ONLY accessed from this task.
        // ----------------------------------------------------
        if (writeStatusLabel && readStatusLabel && statisticsLabel && lastLineLabel)
        {
            char text[128];

            // ------------------------------------------------
            // Write status
            // ------------------------------------------------

            snprintf(text, sizeof(text), "WRITE: %s", lastWriteOK ? "SUCCESS" : "FAIL");
            lv_label_set_text(writeStatusLabel, text);

            // ------------------------------------------------
            // Read status
            // ------------------------------------------------

            snprintf(text, sizeof(text), "READ: %s", lastReadOK ? "SUCCESS" : "FAIL");
            lv_label_set_text(readStatusLabel, text);

            // ------------------------------------------------
            // Last line
            // ------------------------------------------------

            char lineCopy[128];
            if (stateMutex && xSemaphoreTake(stateMutex, 0) == pdTRUE)
            {
                strncpy(lineCopy, lastReadLine, sizeof(lineCopy) - 1);
                lineCopy[sizeof(lineCopy) - 1] = '\0';
                xSemaphoreGive(stateMutex);
                lv_label_set_text(lastLineLabel, lineCopy);
            }
            // ------------------------------------------------
            // Statistics
            // ------------------------------------------------

            snprintf(text, sizeof(text), "WRITE OK: %lu  FAIL: %lu\n"
                                         "READ  OK: %lu  FAIL: %lu",
                     (unsigned long)sdWriteSuccess, (unsigned long)sdWriteFail, (unsigned long)sdReadSuccess, (unsigned long)sdReadFail);
            lv_label_set_text(statisticsLabel, text);
        }

        ++displayFrames;
        if (displayLoadBar)
            lv_bar_set_value(displayLoadBar, (displayFrames / 2) % 101, LV_ANIM_OFF);

        // ----------------------------------------------------
        // LVGL tick / processing interval
        // ----------------------------------------------------

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // Logger
    // --------------------------------------------------------
    Log::init(Log::Level::DEBUG);
    Log::info(Log::DRIVER_TAG, "========================================");
    Log::info(Log::DRIVER_TAG, "ESP32 SD + TFT FREERTOS TEST");
    Log::info(Log::DRIVER_TAG, "========================================");

    // --------------------------------------------------------
    // Deselect SPI devices
    // --------------------------------------------------------
    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);
    pinMode(TFT_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);
    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH);
    Log::info(Log::DRIVER_TAG, "SPI chip-select lines initialized");

    // --------------------------------------------------------
    // SPI for SD
    // --------------------------------------------------------

    if (!SPIBusManager::init(SPI3_SCLK_PIN, SPI3_MISO_PIN, SPI3_MOSI_PIN))
    {
        Log::error(Log::DRIVER_TAG, "FAILED TO INITIALIZE SD SPI BUS");
        return;
    }
    Log::info(Log::DRIVER_TAG, "SD SPI initialized: SCK=%d MISO=%d MOSI=%d CS=%d", SPI3_SCLK_PIN, SPI3_MISO_PIN, SPI3_MOSI_PIN, SD_CS_PIN);

    // --------------------------------------------------------
    // State mutex
    // --------------------------------------------------------

    stateMutex = xSemaphoreCreateMutex();

    if (!stateMutex)
    {
        Log::error(
            Log::DRIVER_TAG,
            "FAILED TO CREATE STATE MUTEX");

        return;
    }

    // ========================================================
    // TFT INIT
    // ========================================================

    Log::info(Log::DISPLAY_TAG, "Initializing TFT...");
    display.init();
    Log::info(Log::DISPLAY_TAG, "TFT initialized");
    Log::info(Log::DISPLAY_TAG, "TFT resolution: %dx%d", display.width(), display.height());

    // ========================================================
    // LVGL INIT
    // ========================================================
    Log::info(Log::DISPLAY_TAG, "Initializing LVGL...");
    lvgl.init(&display, &display);
    createTFTUI();
    Log::info(Log::DISPLAY_TAG, "LVGL initialized");

    // ========================================================
    // SD INIT
    // ========================================================

    Log::info(Log::DRIVER_TAG, "Initializing SD driver...");
    if (!sd.init())
    {
        Log::error(Log::DRIVER_TAG, "SD DRIVER INIT FAILED");
        sdReady = false;
    }
    else
    {
        sdReady = true;
        Log::info(Log::DRIVER_TAG, "SD DRIVER INIT OK");
        Log::info(Log::DRIVER_TAG, "Mounted = %d", sd.mounted());
        Log::info(Log::DRIVER_TAG, "Total = %llu MB", sd.totalBytes() / (1024ULL * 1024ULL));
        Log::info(Log::DRIVER_TAG, "Used = %llu MB", sd.usedBytes() / (1024ULL * 1024ULL));
    }

    // ========================================================
    // CREATE SD STRESS TASK
    // ========================================================
    BaseType_t result = xTaskCreate(sdStressTask, "SD Stress", 12288, nullptr, 2, nullptr);
    if (result != pdPASS)
    {
        Log::error(Log::DRIVER_TAG, "FAILED TO CREATE SD STRESS TASK");
    }

    // ========================================================
    // CREATE TFT TASK
    // ========================================================
    result = xTaskCreate(tftTask, "TFT Task", 8192, nullptr, 3, nullptr);
    if (result != pdPASS)
    {
        Log::error(Log::DISPLAY_TAG, "FAILED TO CREATE TFT TASK");
    }

    // ========================================================
    // READY
    // ========================================================
    Log::info(Log::DRIVER_TAG, "========================================");
    Log::info(Log::DRIVER_TAG, "ALL TEST TASKS STARTED");
    Log::info(Log::DRIVER_TAG, "========================================");
    Log::info(Log::DRIVER_TAG, "Stress commands: 1=FULL 2=ENDURANCE 3=RANDOM 4=FILESYSTEM 5=DISPLAY P=PAUSE R=RESUME");
}

// ============================================================
// LOOP
// ============================================================
void loop()
{
    // All work is performed by FreeRTOS tasks.
    vTaskDelay(portMAX_DELAY);
}
