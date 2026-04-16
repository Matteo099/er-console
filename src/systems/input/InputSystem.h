#pragma once

#include "core/SystemService.h"
#include "drivers/input/INA219Driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include <string>

class InputSystem : public SystemService {
public:
    struct PowerMetrics {
        bool supported = false;
        bool available = false;
        bool overflow = false;
        float busVoltageV = 0.0f;
        float shuntVoltageMv = 0.0f;
        float loadVoltageV = 0.0f;
        float currentMa = 0.0f;
        float powerMw = 0.0f;
        uint32_t sampleTimeMs = 0;
    };

    void init(void *params = nullptr) override;
    void update() override;

    void pinMode(int pin, int mode);  // mode: 0=INPUT, 1=OUTPUT, etc.
    void digitalWrite(int pin, int value);  // value: 0=LOW, 1=HIGH
    int digitalRead(int pin);
    void analogWrite(int pin, int value);  // PWM, if supported

    std::string rfidRead();  // Read RFID badge ID
    void oledPrint(const char* text);  // Print to OLED display

    PowerMetrics getPowerMetrics() const;

private:
    void pollPowerMonitor();

private:
    INA219Driver ina219;
    mutable portMUX_TYPE powerMetricsLock = portMUX_INITIALIZER_UNLOCKED;
    PowerMetrics powerMetrics;
    uint32_t lastPowerPollMs = 0;
    static constexpr uint32_t POWER_POLL_INTERVAL_MS = 500;
};
