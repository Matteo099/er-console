#pragma once

#include <Arduino.h>
#include <Adafruit_INA219.h>

struct INA219Reading
{
    bool available = false;
    bool overflow = false;
    float busVoltageV = 0.0f;
    float shuntVoltageMv = 0.0f;
    float loadVoltageV = 0.0f;
    float currentMa = 0.0f;
    float powerMw = 0.0f;
    uint32_t sampleTimeMs = 0;
};

class INA219Driver
{
public:
    INA219Driver(TwoWire& wire = Wire, uint8_t address = 0x40);

    bool init();
    bool read(INA219Reading& reading);

    bool isAvailable() const { return initialized; }
    uint8_t address() const { return deviceAddress; }

private:
    TwoWire* wireBus = nullptr;
    uint8_t deviceAddress = 0x40;
    bool initialized = false;
    Adafruit_INA219 sensor;
};
