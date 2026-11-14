#pragma once
#include "systems/input/HardwareCheck.h"

class RoomHardwareProbe : public HardwareProbe {
  public:
    bool i2c(uint8_t address) override;
    bool rfid(uint8_t chipSelect, uint8_t reset, bool prepare) override;
    RoomIdentityReading identity(uint8_t pin) override;
};
