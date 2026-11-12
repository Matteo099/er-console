#include "MFRC522Driver.h"
#include <Arduino.h>
#include "log/Logger.h"

#include "drivers/bus/SPIBusLock.h"

MFRC522Driver::MFRC522Driver(SPIClass& spi, uint8_t ssPin, uint8_t rstPin)
    : spiBus(&spi), ssPin(ssPin), rstPin(rstPin), reader(ssPin, rstPin) {
}

bool MFRC522Driver::init() {
    if (!spiBus)
        return false;

    {
        SPIBusLock::Guard guard;
        reader.PCD_Init();
        delay(50);
        byte version = reader.PCD_ReadRegister(MFRC522::VersionReg);
        if (version == 0x00 || version == 0xFF) {
            Log::warn(Log::DRIVER_TAG, "MFRC522 not detected");
            initialized = false;
            return false;
        }
    }

    initialized = true;
    Log::info(Log::DRIVER_TAG, "MFRC522 initialized");
    return true;
}

void MFRC522Driver::shutdown() {
    if (!initialized)
        return;

    {
        SPIBusLock::Guard guard;
        reader.PICC_HaltA();
        reader.PCD_StopCrypto1();
        reader.PCD_AntennaOff();
        reader.PCD_Reset();
    }

    initialized = false;
    Log::info(Log::DRIVER_TAG, "MFRC522 shutdown");
}

bool MFRC522Driver::readTag(RFIDTag& tag) {
    if (!initialized)
        return false;

    SPIBusLock::Guard guard;
    if (!reader.PICC_IsNewCardPresent())
        return false;
    if (!reader.PICC_ReadCardSerial())
        return false;
    tag.uid.clear();
    for (byte i = 0; i < reader.uid.size; i++) {
        char buffer[4];
        snprintf(buffer, sizeof(buffer), "%02X", reader.uid.uidByte[i]);
        tag.uid += buffer;
    }
    tag.valid = true;
    tag.timestampMs = millis();
    reader.PICC_HaltA();
    reader.PCD_StopCrypto1();
    return true;
}

bool MFRC522Driver::isAvailable() const {
    return initialized;
}