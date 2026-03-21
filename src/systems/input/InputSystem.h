#pragma once

#include "core/SystemService.h"
#include <string>

class InputSystem : public SystemService {
public:
    void init(void *params = nullptr) override;
    void update() override;

    void pinMode(int pin, int mode);  // mode: 0=INPUT, 1=OUTPUT, etc.
    void digitalWrite(int pin, int value);  // value: 0=LOW, 1=HIGH
    int digitalRead(int pin);
    void analogWrite(int pin, int value);  // PWM, if supported

    std::string rfidRead();  // Read RFID badge ID
    void oledPrint(const char* text);  // Print to OLED display
};
