#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class I2CBusManager {
  public:
    static bool init(int sdaPin, int sclPin, uint32_t frequency = 400000);

    static void shutdown();

    static TwoWire& bus();

    static SemaphoreHandle_t mutex();

  private:
    static TwoWire wire;
    static SemaphoreHandle_t busMutex;
    static bool initialized;
};