#include <Arduino.h>
#include <SPI.h>

#include "config/BoardConfig.h"
#include "drivers/storage/ESP32SDCardDriver.h"
#include "log/Logger.h"

SPIClass sdSpi(HSPI);

ESP32SDCardDriver sd(sdSpi, SD_CS_PIN);

static EventGroupHandle_t sdEvents = nullptr;

static constexpr EventBits_t SD_READY_BIT = BIT0;
static uint32_t testNumber = 0;
static uint32_t passCount = 0;
static uint32_t failCount = 0;

static bool createHelloFile()
{
    IFile *file =
        sd.open("/hello.txt", FileMode::Write);

    if (!file)
    {
        Log::error(
            Log::DRIVER_TAG,
            "Failed to create /hello.txt");

        return false;
    }

    const char *text =
        "Hello from ESP32SDCardDriver\n";

    file->writeLine(text);

    delete file;

    return true;
}

// ============================================================
// SD STRESS TASK
// ============================================================

static void sdTask(void *parameter)
{
    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "SD TASK STARTED");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    // --------------------------------------------------------
    // Initialize driver
    // --------------------------------------------------------

    Log::info(
        Log::DRIVER_TAG,
        "Initializing SD driver...");

    if (!sd.init())
    {
        Log::error(
            Log::DRIVER_TAG,
            "SD DRIVER INIT FAILED");

        vTaskDelete(nullptr);
        return;
    }

    if (!createHelloFile())
    {
        Log::error(
            Log::DRIVER_TAG,
            "Failed to create hello file");

        vTaskDelete(nullptr);
        return;
    }

    Log::info(
        Log::DRIVER_TAG,
        "SD DRIVER INIT OK");

    xEventGroupSetBits(
        sdEvents,
        SD_READY_BIT);

    Log::info(
        Log::DRIVER_TAG,
        "Mounted = %d",
        sd.mounted());

    Log::info(
        Log::DRIVER_TAG,
        "Total = %llu MB",
        sd.totalBytes() / (1024ULL * 1024ULL));

    Log::info(
        Log::DRIVER_TAG,
        "Used = %llu MB",
        sd.usedBytes() / (1024ULL * 1024ULL));

    // --------------------------------------------------------
    // Stress loop
    // --------------------------------------------------------

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "STARTING SD DRIVER STRESS LOOP");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    while (true)
    {
        ++testNumber;

        char path[64];
        char expected[128];

        snprintf(
            path,
            sizeof(path),
            "/driver_%06lu.txt",
            (unsigned long)testNumber);

        snprintf(
            expected,
            sizeof(expected),
            "ESP32SDCardDriver test #%lu",
            (unsigned long)testNumber);

        Log::info(
            Log::DRIVER_TAG,
            "TEST #%lu: %s",
            (unsigned long)testNumber,
            path);

        // ----------------------------------------------------
        // WRITE
        // ----------------------------------------------------

        IFile *file =
            sd.open(path, FileMode::Write);

        if (!file)
        {
            ++failCount;

            Log::error(
                Log::DRIVER_TAG,
                "OPEN WRITE FAILED: %s",
                path);

            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        /*
         * Use the actual IFile API here.
         *
         * Example:
         *
         * size_t written = file->write(
         *     reinterpret_cast<const uint8_t *>(expected),
         *     strlen(expected));
         *
         * file->flush();
         */

        const size_t expectedLength = strlen(expected);

        size_t written = file->write(
            reinterpret_cast<const uint8_t *>(expected),
            expectedLength);

        if (written != expectedLength)
        {
            ++failCount;

            Log::error(
                Log::DRIVER_TAG,
                "WRITE FAILED: expected=%u written=%u",
                (unsigned)expectedLength,
                (unsigned)written);

            delete file;

            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        file->flush();
        delete file;

        // ----------------------------------------------------
        // READ / VERIFY
        // ----------------------------------------------------

        /*
         * Use the actual IFile API here.
         */

        ++passCount;

        Log::info(
            Log::DRIVER_TAG,
            "PASS #%lu pass=%lu fail=%lu",
            (unsigned long)testNumber,
            (unsigned long)passCount,
            (unsigned long)failCount);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ============================================================
// ALIVE / CONCURRENT READ TASK
// ============================================================

static void aliveTask(void *parameter)
{
    constexpr const char *TEST_FILE = "/hello.txt";

    while (true)
    {

        EventBits_t bits = xEventGroupWaitBits(
            sdEvents,
            SD_READY_BIT,
            pdFALSE, // DON'T clear bit
            pdTRUE,
            pdMS_TO_TICKS(5000));

        if ((bits & SD_READY_BIT) == 0)
        {
            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] SD driver not ready");

            continue;
        }

        Log::info(
            Log::DRIVER_TAG,
            "[ALIVE] I'm alive - reading %s",
            TEST_FILE);

        // ----------------------------------------------------
        // Check driver state
        // ----------------------------------------------------

        if (!sd.mounted())
        {
            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] SD is not mounted");

            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // ----------------------------------------------------
        // Open file through DRIVER
        // ----------------------------------------------------

        IFile *file =
            sd.open(TEST_FILE, FileMode::Read);

        if (!file)
        {
            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] OPEN FAILED: %s",
                TEST_FILE);

            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        // ----------------------------------------------------
        // Read file
        // ----------------------------------------------------

        char buffer[128] = {};

        /*
         * Replace this with your actual IFile API.

         * Example:
         *
         * size_t n = file->read(
         *     reinterpret_cast<uint8_t *>(buffer),
         *     sizeof(buffer) - 1);
         */

        size_t n = file->read(
            reinterpret_cast<uint8_t *>(buffer),
            sizeof(buffer) - 1);

        // ----------------------------------------------------
        // Close
        // ----------------------------------------------------

        delete file;

        Log::info(
            Log::DRIVER_TAG,
            "[ALIVE] Read %u bytes: '%s'",
            (unsigned)n,
            buffer);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Log::init(Log::Level::DEBUG);

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "ESP32SDCardDriver FREERTOS TEST");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    // --------------------------------------------------------
    // Deselect all SPI devices
    // --------------------------------------------------------

    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);

    pinMode(TFT_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);

    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH);

    // --------------------------------------------------------
    // Initialize SPI
    // --------------------------------------------------------

    sdSpi.begin(
        SPI3_SCLK_PIN,
        SPI3_MISO_PIN,
        SPI3_MOSI_PIN,
        SD_CS_PIN);

    Log::info(
        Log::DRIVER_TAG,
        "SD SPI: SCK=%d MISO=%d MOSI=%d CS=%d",
        SPI3_SCLK_PIN,
        SPI3_MISO_PIN,
        SPI3_MOSI_PIN,
        SD_CS_PIN);

    sdEvents = xEventGroupCreate();

    if (!sdEvents)
    {
        Log::error(
            Log::DRIVER_TAG,
            "FAILED TO CREATE SD EVENT GROUP");

        return;
    }

    // --------------------------------------------------------
    // SD task
    // --------------------------------------------------------

    BaseType_t result = xTaskCreate(
        sdTask,
        "SD Task",
        8192,
        nullptr,
        2,
        nullptr);

    if (result != pdPASS)
    {
        Log::error(
            Log::DRIVER_TAG,
            "FAILED TO CREATE SD TASK");

        return;
    }

    // --------------------------------------------------------
    // Alive/read task
    // --------------------------------------------------------

    result = xTaskCreate(
        aliveTask,
        "Alive Task",
        4096,
        nullptr,
        1,
        nullptr);

    if (result != pdPASS)
    {
        Log::error(
            Log::DRIVER_TAG,
            "FAILED TO CREATE ALIVE TASK");

        return;
    }

    Log::info(
        Log::DRIVER_TAG,
        "FreeRTOS tasks started");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    vTaskDelay(portMAX_DELAY);
}