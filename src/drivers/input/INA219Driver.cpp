#include "INA219Driver.h"

#include <cmath>

#include "config/BoardConfig.h"
#include "log/Logger.h"

INA219Driver::INA219Driver(TwoWire& wire, uint8_t address)
    : wireBus(&wire), deviceAddress(address), sensor(address)
{
}

bool INA219Driver::init()
{
    if (!wireBus)
    {
        Log::error(Log::DRIVER_TAG, "INA219 init failed: invalid I2C bus");
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
    Log::info(Log::DRIVER_TAG, "INA219 initialized at 0x%02X", deviceAddress);
    return true;
}

bool INA219Driver::read(INA219Reading& reading)
{
    if (!initialized)
        return false;

    const float shuntVoltageMv = sensor.getShuntVoltage_mV();
    const float busVoltageV = sensor.getBusVoltage_V();
    const float currentMa = sensor.getCurrent_mA();
    const float powerMw = sensor.getPower_mW();

    if (std::isnan(shuntVoltageMv) || std::isnan(busVoltageV) || std::isnan(currentMa) || std::isnan(powerMw))
        return false;

    reading.available = true;
    reading.overflow = false;
    reading.shuntVoltageMv = shuntVoltageMv;
    reading.busVoltageV = busVoltageV;
    reading.currentMa = currentMa;
    reading.powerMw = powerMw;
    reading.loadVoltageV = busVoltageV + (shuntVoltageMv / 1000.0f);
    reading.sampleTimeMs = millis();
    return true;
}
