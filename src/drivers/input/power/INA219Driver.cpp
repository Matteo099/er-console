#include "INA219Driver.h"

#include <cmath>

#include "config/BoardConfig.h"
#include "log/Logger.h"

INA219Driver::INA219Driver(TwoWire &wire, uint8_t address) : wireBus(&wire), deviceAddress(address), sensor(address) {}

bool INA219Driver::init()
{
    if (!wireBus)
    {
        Log::error(Log::DRIVER_TAG, "INA219 invalid wire bus");
        initialized = false;
        return false;
    }
    wireBus->begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
    if (!sensor.begin(wireBus))
    {
        Log::warn(Log::DRIVER_TAG, "INA219 not detected at 0x%02X", deviceAddress);
        initialized = false;
        return false;
    }
    sensor.setCalibration_32V_2A();
    initialized = true;
    Log::info(Log::DRIVER_TAG, "INA219 initialized");
    return true;
}

bool INA219Driver::read(PowerReading &reading)
{
    if (!initialized)
        return false;
    const float shuntVoltageMv = sensor.getShuntVoltage_mV();
    const float busVoltageV = sensor.getBusVoltage_V();
    const float currentMa = sensor.getCurrent_mA();
    const float powerMw = sensor.getPower_mW();
    if (std::isnan(shuntVoltageMv) || std::isnan(busVoltageV) || std::isnan(currentMa) || std::isnan(powerMw))
    {
        return false;
    }
    reading.available = true;
    reading.overflow = false;
    reading.busVoltageV = busVoltageV;
    reading.shuntVoltageMv = shuntVoltageMv;
    reading.loadVoltageV = busVoltageV + shuntVoltageMv / 1000.0f;
    reading.currentMa = currentMa;
    reading.powerMw = powerMw;
    reading.sampleTimeMs = millis();
    return true;
}

bool INA219Driver::isAvailable() const { return initialized; }