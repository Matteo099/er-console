#pragma once
#include "InputConfig.h"

class HardwareProbe {
  public:
    virtual ~HardwareProbe() = default;
    virtual bool i2c(uint8_t address) = 0;
    virtual bool rfid(uint8_t chipSelect, uint8_t reset, bool prepare) = 0;
    virtual std::string identity(uint8_t pin) = 0;
};

namespace HardwareCheck {
bool roomPin(uint8_t pin);
bool validRoomId(const std::string& rom);
std::vector<HardwareIssue> inspect(const InputConfig& config, HardwareProbe& probe, bool prepare);

// A single transient failure does not end a session; two consecutive failures latch a fault.
class Monitor {
  public:
    bool sample(bool healthy) {
        if (healthy)
            failures = 0;
        else if (failures < 2)
            ++failures;
        return failures >= 2;
    }

    void reset() {
        failures = 0;
    }

  private:
    unsigned failures = 0;
};
}  // namespace HardwareCheck
