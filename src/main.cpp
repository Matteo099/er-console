#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "config/BoardConfig.h"
#include "log/Logger.h"

SPIClass sdSpi(HSPI);

void setup()
{
  Log::init(Log::Level::DEBUG);
  Log::info(Log::DRIVER_TAG, "========================================");
  Log::info(Log::DRIVER_TAG, "SD ISOLATION TEST");
  Log::info(Log::DRIVER_TAG, "========================================");

  Log::info(Log::DRIVER_TAG, "SD CS  = %d", SD_CS_PIN);
  Log::info(Log::DRIVER_TAG, "SD SCK = %d", SPI3_SCLK_PIN);
  Log::info(Log::DRIVER_TAG, "SD MISO = %d", SPI3_MISO_PIN);
  Log::info(Log::DRIVER_TAG, "SD MOSI = %d", SPI3_MOSI_PIN);

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

  Log::info(Log::DRIVER_TAG, "Starting SD...");

  uint32_t hz = 4000000;
  if (!SD.begin(
          SD_CS_PIN,
          sdSpi,
          hz,
          "/sd"))
  {
    Log::error(Log::DRIVER_TAG, "SD.begin() FAILED");
    return;
  }

  auto msg = "SD.begin() OK " + String(hz) + "Hz";
  Log::info(Log::DRIVER_TAG, msg.c_str());
}

void loop()
{
    static uint32_t last = 0;
    static uint32_t testNumber = 0;

    if (millis() - last < 1000)
        return;

    last = millis();
    ++testNumber;

    Log::info(
        Log::DRIVER_TAG,
        "========== READ TEST #%lu ==========",
        testNumber);

    File f = SD.open("/sd_test.txt", FILE_READ);

    if (!f)
    {
        Log::error(
            Log::DRIVER_TAG,
            "READ OPEN FAILED");

        return;
    }

    String actual = f.readStringUntil('\n');
    f.close();

    actual.trim();

    Log::info(
        Log::DRIVER_TAG,
        "READ: '%s'",
        actual.c_str());
}