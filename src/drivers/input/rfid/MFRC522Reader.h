#pragma once

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <SPI.h>

#include "IRFIDReader.h"

class MFRC522Reader : public IRFIDReader {
  public:
    MFRC522Reader(SPIClass& spi, uint8_t ssPin, uint8_t rstPin);

    bool init() override;

    void shutdown() override;

    bool readTag(RFIDTag& tag) override;

    bool isAvailable() const override;

  private:
    SPIClass* spiBus = nullptr;
    uint8_t ssPin;
    uint8_t rstPin;

    bool initialized = false;

    MFRC522DriverPinSimple select;
    MFRC522DriverSPI transport;
    MFRC522 reader;
};
