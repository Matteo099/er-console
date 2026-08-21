#include <Arduino.h>
#include <SPI.h>

#include "config/BoardConfig.h"
#include "drivers/storage/ESP32SDCardDriver.h"
#include "drivers/display/TFTDisplay.h"
#include "log/Logger.h"

// ============================================================
// HARDWARE
// ============================================================

SPIClass sdSpi(HSPI);

ESP32SDCardDriver sd(
    sdSpi,
    SD_CS_PIN);

TFTDisplay tft;

// ============================================================
// FREERTOS EVENTS
// ============================================================

static EventGroupHandle_t sdEvents = nullptr;

static constexpr EventBits_t SD_READY_BIT = BIT0;
static constexpr EventBits_t TFT_READY_BIT = BIT1;

// ============================================================
// SD TEST STATISTICS
// ============================================================

static uint32_t testNumber = 0;
static uint32_t passCount = 0;
static uint32_t failCount = 0;

// ============================================================
// TFT STATE
// ============================================================

static volatile bool aliveReadOK = false;

// ============================================================
// CREATE HELLO FILE
// ============================================================

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

    const size_t expectedLength = strlen(text);

    size_t written = file->write(
        reinterpret_cast<const uint8_t *>(text),
        expectedLength);

    if (written != expectedLength)
    {
        Log::error(
            Log::DRIVER_TAG,
            "Failed to write hello file: expected=%u written=%u",
            (unsigned)expectedLength,
            (unsigned)written);

        delete file;
        return false;
    }

    file->flush();

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

        const size_t expectedLength =
            strlen(expected);

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

        IFile *readFile =
            sd.open(path, FileMode::Read);

        if (!readFile)
        {
            ++failCount;

            Log::error(
                Log::DRIVER_TAG,
                "OPEN READ FAILED: %s",
                path);

            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        char buffer[128] = {};

        size_t readBytes = readFile->read(
            reinterpret_cast<uint8_t *>(buffer),
            sizeof(buffer) - 1);

        delete readFile;

        if (readBytes != expectedLength)
        {
            ++failCount;

            Log::error(
                Log::DRIVER_TAG,
                "READ SIZE FAILED: expected=%u read=%u",
                (unsigned)expectedLength,
                (unsigned)readBytes);

            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if (memcmp(
                buffer,
                expected,
                expectedLength) != 0)
        {
            ++failCount;

            Log::error(
                Log::DRIVER_TAG,
                "READ VERIFY FAILED: %s",
                path);

            Log::error(
                Log::DRIVER_TAG,
                "Expected: '%s'",
                expected);

            Log::error(
                Log::DRIVER_TAG,
                "Received: '%s'",
                buffer);

            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // ----------------------------------------------------
        // PASS
        // ----------------------------------------------------

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
    constexpr const char *TEST_FILE =
        "/hello.txt";

    while (true)
    {
        EventBits_t bits =
            xEventGroupWaitBits(
                sdEvents,
                SD_READY_BIT,
                pdFALSE,
                pdTRUE,
                pdMS_TO_TICKS(5000));

        if ((bits & SD_READY_BIT) == 0)
        {
            aliveReadOK = false;

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
            aliveReadOK = false;

            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] SD is not mounted");

            vTaskDelay(
                pdMS_TO_TICKS(5000));

            continue;
        }

        // ----------------------------------------------------
        // Open file
        // ----------------------------------------------------

        IFile *file =
            sd.open(
                TEST_FILE,
                FileMode::Read);

        if (!file)
        {
            aliveReadOK = false;

            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] OPEN FAILED: %s",
                TEST_FILE);

            vTaskDelay(
                pdMS_TO_TICKS(5000));

            continue;
        }

        // ----------------------------------------------------
        // Read file
        // ----------------------------------------------------

        char buffer[128] = {};

        size_t n = file->read(
            reinterpret_cast<uint8_t *>(buffer),
            sizeof(buffer) - 1);

        delete file;

        // ----------------------------------------------------
        // Verify
        // ----------------------------------------------------

        const char *expected =
            "Hello from ESP32SDCardDriver\n";

        const size_t expectedLength =
            strlen(expected);

        if (n != expectedLength ||
            memcmp(
                buffer,
                expected,
                expectedLength) != 0)
        {
            aliveReadOK = false;

            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] VERIFY FAILED");

            Log::error(
                Log::DRIVER_TAG,
                "[ALIVE] Read %u bytes: '%s'",
                (unsigned)n,
                buffer);
        }
        else
        {
            aliveReadOK = true;

            Log::info(
                Log::DRIVER_TAG,
                "[ALIVE] Read %u bytes: '%s'",
                (unsigned)n,
                buffer);
        }

        vTaskDelay(
            pdMS_TO_TICKS(5000));
    }
}

// ============================================================
// TFT TASK
// ============================================================

static void tftTask(void *parameter)
{
    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    Log::info(
        Log::DISPLAY_TAG,
        "TFT TASK STARTED");

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    // --------------------------------------------------------
    // Initialize TFT
    // --------------------------------------------------------

    Log::info(
        Log::DISPLAY_TAG,
        "Initializing TFT...");

    tft.init();

    Log::info(
        Log::DISPLAY_TAG,
        "TFT INIT OK");

    Log::info(
        Log::DISPLAY_TAG,
        "TFT resolution: %dx%d",
        tft.width(),
        tft.height());

    xEventGroupSetBits(
        sdEvents,
        TFT_READY_BIT);

    // --------------------------------------------------------
    // TFT update loop
    // --------------------------------------------------------

    while (true)
    {
        // ----------------------------------------------------
        // Wait for SD
        // ----------------------------------------------------

        EventBits_t bits =
            xEventGroupGetBits(sdEvents);

        bool sdReady =
            (bits & SD_READY_BIT) != 0;

        // ----------------------------------------------------
        // Gather current state
        // ----------------------------------------------------

        bool mounted =
            sd.mounted();

        uint32_t currentTest =
            testNumber;

        uint32_t currentPass =
            passCount;

        uint32_t currentFail =
            failCount;

        bool aliveOK =
            aliveReadOK;

        // ----------------------------------------------------
        // Display
        // ----------------------------------------------------

        //
        // NOTE:
        // We deliberately keep this simple and use the
        // underlying LovyanGFX instance through the TFT
        // driver's current public API only where available.
        //
        // The existing TFTDisplay abstraction does not expose
        // text drawing, so the task currently verifies TFT
        // initialization and keeps the task alive.
        //
        // The next step can expose a small drawText()/clear()
        // API from TFTDisplay.
        //

        Log::info(
            Log::DISPLAY_TAG,
            "[TFT] SD=%s mounted=%s test=%lu pass=%lu fail=%lu alive=%s",
            sdReady ? "READY" : "NOT_READY",
            mounted ? "YES" : "NO",
            (unsigned long)currentTest,
            (unsigned long)currentPass,
            (unsigned long)currentFail,
            aliveOK ? "OK" : "FAIL");

        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Log::init(
        Log::Level::DEBUG);

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "ESP32 SD + TFT FREERTOS TEST");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    // --------------------------------------------------------
    // Deselect all SPI devices
    // --------------------------------------------------------

    pinMode(
        SD_CS_PIN,
        OUTPUT);

    digitalWrite(
        SD_CS_PIN,
        HIGH);

    pinMode(
        TFT_CS_PIN,
        OUTPUT);

    digitalWrite(
        TFT_CS_PIN,
        HIGH);

    pinMode(
        TOUCH_CS_PIN,
        OUTPUT);

    digitalWrite(
        TOUCH_CS_PIN,
        HIGH);

    // --------------------------------------------------------
    // Initialize SD SPI
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

    // --------------------------------------------------------
    // Create event group
    // --------------------------------------------------------

    sdEvents =
        xEventGroupCreate();

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

    BaseType_t result =
        xTaskCreate(
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

    result =
        xTaskCreate(
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

    // --------------------------------------------------------
    // TFT task
    // --------------------------------------------------------

    result =
        xTaskCreate(
            tftTask,
            "TFT Task",
            8192,
            nullptr,
            1,
            nullptr);

    if (result != pdPASS)
    {
        Log::error(
            Log::DISPLAY_TAG,
            "FAILED TO CREATE TFT TASK");

        return;
    }

    // --------------------------------------------------------
    // Done
    // --------------------------------------------------------

    Log::info(
        Log::DRIVER_TAG,
        "FreeRTOS tasks started");

    Log::info(
        Log::DRIVER_TAG,
        "SD Task     : priority 2");

    Log::info(
        Log::DRIVER_TAG,
        "Alive Task  : priority 1");

    Log::info(
        Log::DRIVER_TAG,
        "TFT Task    : priority 1");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    vTaskDelay(
        portMAX_DELAY);
}