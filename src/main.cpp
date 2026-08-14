#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "config/BoardConfig.h"
#include "log/Logger.h"

SPIClass sdSpi(HSPI);

static bool sdReady = false;

static uint32_t testNumber = 0;
static uint32_t passCount = 0;
static uint32_t failCount = 0;

static constexpr uint32_t SD_HZ = 1000000;

void setup()
{
    Log::init(Log::Level::DEBUG);

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "SD STRESS TEST");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "SD CS   = %d",
        SD_CS_PIN);

    Log::info(
        Log::DRIVER_TAG,
        "SD SCK  = %d",
        SPI3_SCLK_PIN);

    Log::info(
        Log::DRIVER_TAG,
        "SD MISO = %d",
        SPI3_MISO_PIN);

    Log::info(
        Log::DRIVER_TAG,
        "SD MOSI = %d",
        SPI3_MOSI_PIN);

    // --------------------------------------------------
    // Deselect all SPI devices
    // --------------------------------------------------

    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);

    pinMode(TFT_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);

    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH);

    // --------------------------------------------------
    // SPI
    // --------------------------------------------------

    sdSpi.begin(
        SPI3_SCLK_PIN,
        SPI3_MISO_PIN,
        SPI3_MOSI_PIN,
        SD_CS_PIN);

    Log::info(
        Log::DRIVER_TAG,
        "SD begin @ %lu Hz",
        SD_HZ);

    // --------------------------------------------------
    // SD
    // --------------------------------------------------

    sdReady = SD.begin(
        SD_CS_PIN,
        sdSpi,
        SD_HZ,
        "/sd");

    if (!sdReady)
    {
        Log::error(
            Log::DRIVER_TAG,
            "SD.begin FAILED");

        return;
    }

    Log::info(
        Log::DRIVER_TAG,
        "SD.begin OK");

    Log::info(
        Log::DRIVER_TAG,
        "Card type = %d",
        SD.cardType());

    Log::info(
        Log::DRIVER_TAG,
        "Card size = %llu MB",
        SD.cardSize() / (1024ULL * 1024ULL));

    Log::info(
        Log::DRIVER_TAG,
        "Total     = %llu MB",
        SD.totalBytes() / (1024ULL * 1024ULL));

    Log::info(
        Log::DRIVER_TAG,
        "Used      = %llu MB",
        SD.usedBytes() / (1024ULL * 1024ULL));

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "STARTING STRESS LOOP");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");
}

void loop()
{
    if (!sdReady)
    {
        delay(1000);
        return;
    }

    ++testNumber;

    char path[64];
    char expected[256];

    snprintf(
        path,
        sizeof(path),
        "/stress_%06lu.txt",
        (unsigned long)testNumber);

    snprintf(
        expected,
        sizeof(expected),
        "SD STRESS TEST\n"
        "Iteration: %lu\n"
        "Timestamp: %lu\n"
        "Payload: ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789\n",
        (unsigned long)testNumber,
        (unsigned long)millis());

    Log::info(
        Log::DRIVER_TAG,
        "TEST #%lu: %s",
        (unsigned long)testNumber,
        path);

    // ==================================================
    // WRITE
    // ==================================================

    File f = SD.open(path, FILE_WRITE);

    if (!f)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "FAIL #%lu: OPEN WRITE",
            (unsigned long)testNumber);

        delay(1000);
        return;
    }

    size_t expectedLength = strlen(expected);

    size_t written = f.print(expected);

    if (written != expectedLength)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "FAIL #%lu: WRITE returned %u expected %u",
            (unsigned long)testNumber,
            (unsigned)written,
            (unsigned)expectedLength);

        f.close();

        delay(1000);
        return;
    }

    f.flush();
    f.close();

    // ==================================================
    // READ BACK
    // ==================================================

    f = SD.open(path, FILE_READ);

    if (!f)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "FAIL #%lu: OPEN READ",
            (unsigned long)testNumber);

        delay(1000);
        return;
    }

    size_t fileSize = f.size();

    char actual[256] = {};

    size_t readCount = f.readBytes(
        actual,
        sizeof(actual) - 1);

    f.close();

    actual[readCount] = '\0';

    // ==================================================
    // VERIFY SIZE
    // ==================================================

    if (fileSize != expectedLength)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "FAIL #%lu: SIZE expected=%u actual=%u",
            (unsigned long)testNumber,
            (unsigned)expectedLength,
            (unsigned)fileSize);

        delay(1000);
        return;
    }

    // ==================================================
    // VERIFY CONTENT
    // ==================================================

    if (readCount != expectedLength)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "FAIL #%lu: READ expected=%u actual=%u",
            (unsigned long)testNumber,
            (unsigned)expectedLength,
            (unsigned)readCount);

        delay(1000);
        return;
    }

    if (memcmp(actual, expected, expectedLength) != 0)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "FAIL #%lu: DATA MISMATCH",
            (unsigned long)testNumber);

        Log::error(
            Log::DRIVER_TAG,
            "Expected: '%s'",
            expected);

        Log::error(
            Log::DRIVER_TAG,
            "Actual:   '%s'",
            actual);

        delay(1000);
        return;
    }

    // ==================================================
    // SUCCESS
    // ==================================================

    ++passCount;

    Log::info(
        Log::DRIVER_TAG,
        "PASS #%lu  size=%u  pass=%lu fail=%lu",
        (unsigned long)testNumber,
        (unsigned)fileSize,
        (unsigned long)passCount,
        (unsigned long)failCount);

    delay(100);
}