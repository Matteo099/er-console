#include "MFRC522Driver.h"
#include <Arduino.h>
#include "log/Logger.h"

MFRC522Driver::MFRC522Driver(uint8_t ssPin, uint8_t rstPin) : ssPin(ssPin), rstPin(rstPin), reader(ssPin, rstPin) {}

bool MFRC522Driver::init()
{
    SPI.begin();
    reader.PCD_Init();
    delay(50);
    const byte version = reader.PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF)
    {
        Log::warn(Log::DRIVER_TAG, "MFRC522 not detected");
        initialized = false;
        return false;
    }
    initialized = true;
    Log::info(Log::DRIVER_TAG, "MFRC522 initialized");
    return true;
}

void MFRC522Driver::shutdown()
{
    if (!initialized)
        return;

    // Stop any active RFID communication
    reader.PICC_HaltA();
    reader.PCD_StopCrypto1();

    // Disable antenna / RF field
    reader.PCD_AntennaOff();

    // Reset MFRC522 registers to default state
    reader.PCD_Reset();

    // Release SPI pins only if this driver owns SPI
    // SPI.end();

    initialized = false;

    Log::info(Log::DRIVER_TAG, "MFRC522 shutdown");
}

bool MFRC522Driver::readTag(RFIDTag &tag)
{
    if (!initialized)
        return false;
    if (!reader.PICC_IsNewCardPresent())
        return false;
    if (!reader.PICC_ReadCardSerial())
        return false;
    tag.uid.clear();
    for (byte i = 0; i < reader.uid.size; ++i)
    {
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

bool MFRC522Driver::isAvailable() const { return initialized; }