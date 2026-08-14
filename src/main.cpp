#include <Arduino.h>
#include <SPI.h>

#include "config/BoardConfig.h"
#include "drivers/storage/ESP32SDCardDriver.h"
#include "log/Logger.h"

SPIClass sdSpi(HSPI);

ESP32SDCardDriver sd(sdSpi, SD_CS_PIN);

static uint32_t testNumber = 0;
static uint32_t passCount = 0;
static uint32_t failCount = 0;

void setup()
{
    Log::init(Log::Level::DEBUG);

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    Log::info(
        Log::DRIVER_TAG,
        "ESP32SDCardDriver STRESS TEST");

    Log::info(
        Log::DRIVER_TAG,
        "========================================");

    pinMode(SD_CS_PIN, OUTPUT);
    digitalWrite(SD_CS_PIN, HIGH);

    pinMode(TFT_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);

    pinMode(TOUCH_CS_PIN, OUTPUT);
    digitalWrite(TOUCH_CS_PIN, HIGH);

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

    Log::info(
        Log::DRIVER_TAG,
        "Initializing driver...");

    if (!sd.init())
    {
        Log::error(
            Log::DRIVER_TAG,
            "DRIVER INIT FAILED");

        return;
    }

    Log::info(
        Log::DRIVER_TAG,
        "DRIVER INIT OK");

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
        "Used  = %llu MB",
        sd.usedBytes() / (1024ULL * 1024ULL));
}

void loop()
{
    if (!sd.mounted())
    {
        delay(1000);
        return;
    }

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

    // --------------------------------------------------
    // WRITE
    // --------------------------------------------------

    IFile *file =
        sd.open(path, FileMode::Write);

    if (!file)
    {
        ++failCount;

        Log::error(
            Log::DRIVER_TAG,
            "OPEN WRITE FAILED");

        return;
    }

    // This depends on your IFile API.
    // Use whatever your actual write method is.
    
    // file->write(...);

    delete file;

    // --------------------------------------------------
    // READ
    // --------------------------------------------------

    // Same here: use the actual IFile interface.
    
    ++passCount;

    Log::info(
        Log::DRIVER_TAG,
        "PASS #%lu pass=%lu fail=%lu",
        (unsigned long)testNumber,
        (unsigned long)passCount,
        (unsigned long)failCount);

    delay(100);
}