#pragma once

#include <vector>
#include <string>

#include "ISubsystem.h"

#include "drivers/input/sevensegment/ISevenSegmentDriver.h"

class SevenSegmentSystem : public ISubsystem {
  public:
    void registerDriver(ISevenSegmentDriver* driver);
    bool init() override;
    void update() override;
    void display(uint8_t display, const std::string& value);
    void clear(uint8_t display);
    void brightness(uint8_t display, uint8_t value);
    void shutdown() override;

  private:
    std::vector<ISevenSegmentDriver*> drivers;
};