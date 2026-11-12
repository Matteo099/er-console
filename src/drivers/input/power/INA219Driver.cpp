#include "INA219Driver.h"

#include <cmath>

#include "config/BoardConfig.h"
#include "log/Logger.h"
#include "drivers/bus/I2CBusLock.h"

INA219Driver::INA219Driver(TwoWire& wire, uint8_t address)
    : wireBus(&wire), deviceAddress(address), sensor(address) {
}

bool INA219Driver::init() {
    I2CBusLock::Guard guard;

    if (!sensor.begin(wireBus)) {
        Log::warn(Log::DRIVER_TAG, "INA219 not detected at 0x%02X", deviceAddress);
        initialized = false;
        return false;
    }
    sensor.setCalibration_32V_2A();
    initialized = true;
    Log::info(Log::DRIVER_TAG, "INA219 initialized");
    return true;
}

void INA219Driver::shutdown() {
    if (!initialized)
        return;

    // No specific shutdown needed for INA219, but we can reset the sensor if desired
    initialized = false;
    Log::info(Log::DRIVER_TAG, "INA219 shutdown");
}

bool INA219Driver::read(PowerReading& reading) {
    if (!initialized)
        return false;

    I2CBusLock::Guard guard;

    const float shuntVoltageMv = sensor.getShuntVoltage_mV();
    const float busVoltageV = sensor.getBusVoltage_V();
    const float currentMa = sensor.getCurrent_mA();
    const float powerMw = sensor.getPower_mW();
    if (std::isnan(shuntVoltageMv) || std::isnan(busVoltageV) || std::isnan(currentMa) ||
        std::isnan(powerMw)) {
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

bool INA219Driver::isAvailable() const {
    return initialized;
}