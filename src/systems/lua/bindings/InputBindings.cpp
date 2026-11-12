#include "InputBindings.h"

#include "systems/input/InputSystem.h"

#include "ChannelBindings.h"
#include "RFIDBindings.h"
#include "OLEDBindings.h"
#include "RelayBindings.h"
#include "RGBBindings.h"
#include "SevenSegmentBindings.h"
#include "log/Logger.h"

InputBindings::InputBindings(InputSystem* inputSystem) : input(inputSystem) {
}

void InputBindings::registerAPI(lua_State* L) {
    if (!input) {
        Log::warn(Log::LUA_TAG, "InputSystem is nullptr, cannot register input bindings");
        return;
    }

    if (auto* channels = input->channels()) {
        Log::debug(Log::LUA_TAG, "Registering InputBindings for channels");
        ChannelBindings(channels).registerAPI(L);
    }
    if (auto* rfid = input->rfid()) {
        Log::debug(Log::LUA_TAG, "Registering InputBindings for RFID");
        RFIDBindings(rfid).registerAPI(L);
    }
    if (auto* oled = input->oled()) {
        Log::debug(Log::LUA_TAG, "Registering InputBindings for OLED");
        OLEDBindings(oled).registerAPI(L);
    }
    if (auto* relays = input->relays()) {
        Log::debug(Log::LUA_TAG, "Registering InputBindings for Relays");
        RelayBindings(relays).registerAPI(L);
    }
    if (auto* rgb = input->rgb()) {
        Log::debug(Log::LUA_TAG, "Registering InputBindings for RGB");
        RGBBindings(rgb).registerAPI(L);
    }
    if (auto* sevenSegment = input->sevenSegment()) {
        Log::debug(Log::LUA_TAG, "Registering InputBindings for Seven Segment");
        SevenSegmentBindings(sevenSegment).registerAPI(L);
    }
}