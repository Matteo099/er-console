#pragma once

#include <vector>

#include "ISubsystem.h"
#include "ChannelSystem.h"

class RelaySystem : public ISubsystem {
  public:
    RelaySystem(ChannelSystem* channels);
    bool init() override;
    void update() override;
    void shutdown() override;
    void registerRelay(uint8_t id, uint16_t channel, bool activeLow = false);
    void set(uint8_t relay, bool state);
    bool get(uint8_t relay) const;
    void toggle(uint8_t relay);
    void pulse(uint8_t relay, uint32_t durationMs);

  private:
    struct RelayConfig {
        uint16_t channel;
        bool activeLow;
        bool state;
        bool pulsing;
        uint32_t pulseEnd;
    };

    ChannelSystem* channels = nullptr;
    std::vector<RelayConfig> relays;
};