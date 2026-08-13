#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config/BoardConfig.h"

class SPIBusManager
{
public:
    static bool init(
        int sclkPin = SPI3_SCLK_PIN,
        int misoPin = SPI3_MISO_PIN,
        int mosiPin = SPI3_MOSI_PIN);

    static void shutdown();

    static SPIClass &bus();

    static SemaphoreHandle_t mutex();

private:
    static SPIClass spi;

    static SemaphoreHandle_t busMutex;

    static bool initialized;
};