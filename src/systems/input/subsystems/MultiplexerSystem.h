#pragma once

#include <vector>

#include "drivers/input/multiplexer/IMultiplexer.h"
#include "ISubsystem.h"

class MultiplexerSystem : public ISubsystem {
  public:
    void registerMultiplexer(IMultiplexer* mux);

    bool init() override;

    void update() override;

    void shutdown() override;

    bool select(uint8_t mux, uint8_t channel);

    int analogRead(uint8_t mux, uint8_t channel);

    bool digitalRead(uint8_t mux, uint8_t channel);

    void digitalWrite(uint8_t mux, uint8_t channel, bool value);

  private:
    std::vector<IMultiplexer*> multiplexers;
};