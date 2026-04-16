#include "InputSystem.h"
#include <Arduino.h>  // Assuming Arduino framework for GPIO
// #include <Wire.h>     // For I2C (OLED)
// #include <Adafruit_SSD1306.h>  // Assuming OLED library
// #include <MFRC522.h>  // Assuming RFID library

#include "log/Logger.h"

// Mock or actual pins
// #define RFID_SS_PIN 10
// #define RFID_RST_PIN 9
// #define OLED_RESET -1
// #define SCREEN_ADDRESS 0x3C

// MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
// Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

void InputSystem::init(void *params) {
    // Initialize GPIO, RFID, OLED

    // SPI.begin();
    // rfid.PCD_Init();
    
    // if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //     Serial.println(F("SSD1306 allocation failed"));
    // }
    // display.display();
    // delay(2000);
    // display.clearDisplay();

    powerMetrics = PowerMetrics{};
    powerMetrics.supported = ina219.init();
    if (!powerMetrics.supported) {
        Log::warn(Log::INPUT_TAG, "Power monitor (INA219) not available; continuing without telemetry");
    } else {
        Log::info(Log::INPUT_TAG, "Power monitor (INA219) initialized");
    }
}

void InputSystem::update() {
    pollPowerMonitor();
}

void InputSystem::pinMode(int pin, int mode) {
    ::pinMode(pin, mode);
}

void InputSystem::digitalWrite(int pin, int value) {
    ::digitalWrite(pin, value);
}

int InputSystem::digitalRead(int pin) {
    return ::digitalRead(pin);
}

void InputSystem::analogWrite(int pin, int value) {
    ::analogWrite(pin, value);
}

std::string InputSystem::rfidRead() {
    // if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    //     return "";
    // }
    
    // String id = "";
    // for (byte i = 0; i < rfid.uid.size; i++) {
    //     id += String(rfid.uid.uidByte[i], HEX);
    // }
    // rfid.PICC_HaltA();
    // return id.c_str();
    return "mocked_rfid_id";
}

void InputSystem::oledPrint(const char* text) {
    // display.clearDisplay();
    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(0,0);
    // display.println(text);
    // display.display();
}

InputSystem::PowerMetrics InputSystem::getPowerMetrics() const {
    taskENTER_CRITICAL(&powerMetricsLock);
    PowerMetrics copy = powerMetrics;
    taskEXIT_CRITICAL(&powerMetricsLock);
    return copy;
}

void InputSystem::pollPowerMonitor() {
    if (!powerMetrics.supported)
        return;

    const uint32_t now = millis();
    if (now - lastPowerPollMs < POWER_POLL_INTERVAL_MS)
        return;

    lastPowerPollMs = now;

    INA219Reading reading;
    if (!ina219.read(reading))
        return;

    taskENTER_CRITICAL(&powerMetricsLock);
    powerMetrics.available = reading.available;
    powerMetrics.overflow = reading.overflow;
    powerMetrics.busVoltageV = reading.busVoltageV;
    powerMetrics.shuntVoltageMv = reading.shuntVoltageMv;
    powerMetrics.loadVoltageV = reading.loadVoltageV;
    powerMetrics.currentMa = reading.currentMa;
    powerMetrics.powerMw = reading.powerMw;
    powerMetrics.sampleTimeMs = reading.sampleTimeMs;
    taskEXIT_CRITICAL(&powerMetricsLock);
}
