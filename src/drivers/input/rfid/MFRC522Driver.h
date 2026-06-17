#pragma once

#include <MFRC522.h>
#include <SPI.h>

#include "IRFIDReader.h"

class MFRC522Driver : public IRFIDReader
{
public:
    MFRC522Driver(
        uint8_t ssPin,
        uint8_t rstPin);

    bool init() override;

    bool readTag(RFIDTag& tag) override;

    bool isAvailable() const override;

private:
    uint8_t ssPin;
    uint8_t rstPin;

    bool initialized = false;

    MFRC522 reader;
};