#pragma once

#include <vector>

#include "drivers/input/channel/IChannelProvider.h"
#include "ISubsystem.h"

class ChannelSystem : public ISubsystem {
  public:
    void registerProvider(IChannelProvider* provider);
    bool init() override;
    void shutdown() override;
    void update() override;
    void pinMode(uint16_t channel, uint8_t mode);
    void digitalWrite(uint16_t channel, bool state);
    bool digitalRead(uint16_t channel);
    int analogRead(uint16_t channel);
    void analogWrite(uint16_t channel, int value);

  private:
    IChannelProvider* findProvider(uint16_t channel);
    std::vector<IChannelProvider*> providers;
};