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

// ============================================================
// SD STRESS TEST
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
  // SPI initialization
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
  // SD driver initialization
  // --------------------------------------------------------

  Log::info(
      Log::DRIVER_TAG,
      "Initializing SD driver...");

  if (!sd.init())
  {
    Log::error(
        Log::DRIVER_TAG,
        "SD DRIVER INIT FAILED");

    // Task has nothing useful left to do.
    vTaskDelete(nullptr);
    return;
  }

  Log::info(
      Log::DRIVER_TAG,
      "SD DRIVER INIT OK");

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

    // ====================================================
    // WRITE
    // ====================================================

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
     * TODO:
     *
     * Replace this with the actual IFile write API.
     *
     * For example, if your interface is:
     *
     *     file->write(const uint8_t *, size_t);
     *
     * then:
     *
     *     size_t written =
     *         file->write(
     *             (const uint8_t *)expected,
     *             strlen(expected));
     *
     * and optionally:
     *
     *     file->flush();
     */

    delete file;

    // ====================================================
    // READ
    // ====================================================

    /*
     * TODO:
     *
     * Use the actual IFile read API here.
     *
     * The important part of the test is:
     *
     *     write
     *       ↓
     *     close
     *       ↓
     *     reopen
     *       ↓
     *     read
     *       ↓
     *     compare
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
// ALIVE TASK
// ============================================================

static void aliveTask(void *parameter)
{
  while (true)
  {
    Log::info(
        Log::DRIVER_TAG,
        "I'm alive");

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
  // Deselect all SPI devices before starting tasks.
  // --------------------------------------------------------

  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);

  pinMode(TFT_CS_PIN, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);

  pinMode(TOUCH_CS_PIN, OUTPUT);
  digitalWrite(TOUCH_CS_PIN, HIGH);

  // --------------------------------------------------------
  // Create SD task
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
  // Create alive task
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
  // Everything is handled by FreeRTOS tasks.
  vTaskDelay(portMAX_DELAY);
}