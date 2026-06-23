#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class SPIBusManager
{
public:
    static bool init(
        int sclkPin = -1,
        int misoPin = -1,
        int mosiPin = -1);

    static void shutdown();

    static SPIClass &bus();

    static SemaphoreHandle_t mutex();

private:
    static SPIClass spi;

    static SemaphoreHandle_t busMutex;

    static bool initialized;
};