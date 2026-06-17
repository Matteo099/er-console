#pragma once

#include <Arduino.h>
#include <Adafruit_INA219.h>

#include "IPowerMonitor.h"

class INA219Driver : public IPowerMonitor
{
public:
    explicit INA219Driver(
        TwoWire &wire = Wire,
        uint8_t address = 0x40);

    bool init() override;

    bool read(PowerReading &reading) override;

    bool isAvailable() const override;

private:
    TwoWire *wireBus = nullptr;

    uint8_t deviceAddress = 0x40;

    bool initialized = false;

    Adafruit_INA219 sensor;
};