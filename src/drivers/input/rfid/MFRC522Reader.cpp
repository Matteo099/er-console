#include "MFRC522Reader.h"
#include <Arduino.h>
#include "log/Logger.h"

#include "drivers/bus/SPIBusLock.h"

MFRC522Reader::MFRC522Reader(SPIClass& spi, uint8_t ssPin, uint8_t rstPin)
    : spiBus(&spi), ssPin(ssPin), rstPin(rstPin), select(ssPin), transport(select, spi),
      reader(transport) {
}

bool MFRC522Reader::init() {
    if (!spiBus)
        return false;

    {
        SPIBusLock::Guard guard;
        pinMode(rstPin, OUTPUT);
        digitalWrite(rstPin, HIGH);
        if (!reader.PCD_Init())
            return false;
        delay(50);
        byte version = static_cast<byte>(reader.PCD_GetVersion());
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

void MFRC522Reader::shutdown() {
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

bool MFRC522Reader::readTag(RFIDTag& tag) {
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

bool MFRC522Reader::isAvailable() const {
    return initialized;
}
