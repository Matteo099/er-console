#include "LuaConstantsBinding.h"

#include <Arduino.h>

// ============================================================
// Pin modes
// ============================================================

// @enum PinMode
// @value INPUT             0x01
// @value OUTPUT            0x03
// @value PULLUP            0x04
// @value INPUT_PULLUP      0x05
// @value PULLDOWN          0x08
// @value INPUT_PULLDOWN    0x09
// @value OPEN_DRAIN        0x10
// @value OUTPUT_OPEN_DRAIN 0x13
// @value ANALOG            0xC0
void LuaConstantsBinding::registerPinMode(lua_State *L)
{
    lua_newtable(L);
    lua_pushinteger(L, INPUT);
    lua_setfield(L, -2, "INPUT");
    lua_pushinteger(L, OUTPUT);
    lua_setfield(L, -2, "OUTPUT");
    lua_pushinteger(L, INPUT_PULLUP);
    lua_setfield(L, -2, "INPUT_PULLUP");
    lua_pushinteger(L, PULLUP);
    lua_setfield(L, -2, "PULLUP");
    lua_pushinteger(L, INPUT_PULLDOWN);
    lua_setfield(L, -2, "INPUT_PULLDOWN");
    lua_pushinteger(L, PULLDOWN);
    lua_setfield(L, -2, "PULLDOWN");
    lua_pushinteger(L, OPEN_DRAIN);
    lua_setfield(L, -2, "OPEN_DRAIN");
    lua_pushinteger(L, OUTPUT_OPEN_DRAIN);
    lua_setfield(L, -2, "OUTPUT_OPEN_DRAIN");
    lua_pushinteger(L, ANALOG);
    lua_setfield(L, -2, "ANALOG");
    lua_setglobal(L, "PinMode");
}

// ============================================================
// Digital values
// ============================================================

// @enum DigitalValue
// @value LOW 0x0
// @value HIGH 0x1
void LuaConstantsBinding::registerDigitalValue(lua_State *L)
{
    lua_newtable(L);
    lua_pushinteger(L, LOW);
    lua_setfield(L, -2, "LOW");
    lua_pushinteger(L, HIGH);
    lua_setfield(L, -2, "HIGH");
    lua_setglobal(L, "DigitalValue");
}

// ============================================================
// Trigger modes
// ============================================================

// @enum TriggerMode
// @value RISING 0x01
// @value FALLING 0x02
// @value CHANGE 0x03
void LuaConstantsBinding::registerTriggerMode(lua_State *L)
{
    lua_newtable(L);
    lua_pushinteger(L, RISING);
    lua_setfield(L, -2, "RISING");
    lua_pushinteger(L, FALLING);
    lua_setfield(L, -2, "FALLING");
    lua_pushinteger(L, CHANGE);
    lua_setfield(L, -2, "CHANGE");
    lua_setglobal(L, "TriggerMode");
}

// ============================================================
// Game object content types
// ============================================================

// @enum GameObjectContentType
// @value TEXT  Text paragraph content
// @value IMAGE Image content loaded from storage path
void LuaConstantsBinding::registerGameObjectContentType(lua_State *L)
{
    lua_newtable(L);

    lua_pushstring(L, "text");
    lua_setfield(L, -2, "TEXT");

    lua_pushstring(L, "image");
    lua_setfield(L, -2, "IMAGE");

    lua_setglobal(L, "GameObjectContentType");
}

// ============================================================
// Main registration
// ============================================================

void LuaConstantsBinding::registerAPI(lua_State *L)
{
    registerPinMode(L);
    registerDigitalValue(L);
    registerTriggerMode(L);
    registerGameObjectContentType(L);
}