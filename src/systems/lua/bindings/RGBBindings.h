#pragma once

#include "LuaBinding.h"

class RgbLedSystem;

class RGBBindings : public LuaBinding
{
public:
    explicit RGBBindings(RgbLedSystem *system);
    void registerAPI(lua_State *L) override;

private:
    RgbLedSystem *rgb;
    static RgbLedSystem *get(lua_State *L);
    static int setColor(lua_State *L);
    static int setBrightness(lua_State *L);
};