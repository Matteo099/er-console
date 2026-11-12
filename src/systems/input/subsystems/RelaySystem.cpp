#include "RelaySystem.h"

#include <Arduino.h>

RelaySystem::RelaySystem(ChannelSystem* channels) : channels(channels) {
}

bool RelaySystem::init() {
    for (auto& relay : relays) {
        channels->pinMode(relay.channel, OUTPUT);
        set(&relay - &relays[0], false);
    }
    return true;
}

void RelaySystem::update() {
    uint32_t now = millis();
    for (uint8_t i = 0; i < relays.size(); i++) {
        auto& relay = relays[i];
        if (relay.pulsing && now >= relay.pulseEnd) {
            set(i, false);
            relay.pulsing = false;
        }
    }
}

void RelaySystem::shutdown() {
    for (auto& relay : relays) {
        channels->digitalWrite(relay.channel, relay.activeLow ? HIGH : LOW);
    }
}

void RelaySystem::registerRelay(uint8_t id, uint16_t channel, bool activeLow) {
    if (id >= relays.size())
        relays.resize(id + 1);
    relays[id].channel = channel;
    relays[id].activeLow = activeLow;
    relays[id].state = false;
}

void RelaySystem::set(uint8_t relay, bool state) {
    if (relay >= relays.size())
        return;
    auto& r = relays[relay];
    r.state = state;
    bool output = r.activeLow ? !state : state;
    channels->digitalWrite(r.channel, output);
}

bool RelaySystem::get(uint8_t relay) const {
    if (relay >= relays.size())
        return false;
    return relays[relay].state;
}

void RelaySystem::toggle(uint8_t relay) {
    set(relay, !get(relay));
}

void RelaySystem::pulse(uint8_t relay, uint32_t durationMs) {
    if (relay >= relays.size())
        return;
    set(relay, true);
    relays[relay].pulsing = true;
    relays[relay].pulseEnd = millis() + durationMs;
}