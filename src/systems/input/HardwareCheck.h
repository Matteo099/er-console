#pragma once
#include "InputConfig.h"

struct RoomIdentityReading {
    unsigned millivolts = 0;
    unsigned spreadMillivolts = 0;
};

class HardwareProbe {
  public:
    virtual ~HardwareProbe() = default;
    virtual bool i2c(uint8_t address) = 0;
    virtual bool rfid(uint8_t chipSelect, uint8_t reset, bool prepare) = 0;
    virtual RoomIdentityReading identity(uint8_t pin) = 0;
};

namespace HardwareCheck {
bool roomPin(uint8_t pin);
// Console-side 1% pull-up to 3.3 V; the room resistor connects the ADC line to GND.
constexpr unsigned IdentityPullupOhms = 1000;
constexpr unsigned IdentitySupplyMillivolts = 3300;
constexpr unsigned IdentityShortMillivolts = 120;
constexpr unsigned IdentityOpenMillivolts = 3000;
constexpr unsigned IdentityMaxSpreadMillivolts = 80;
constexpr unsigned IdentityTolerancePercent = 10;
constexpr unsigned IdentityAdcMarginMillivolts = 25;
bool identityPin(uint8_t pin);
unsigned identityVoltage(unsigned resistanceOhms);
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
