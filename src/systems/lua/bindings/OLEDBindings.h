#pragma once

#include "LuaBinding.h"

class OLEDSystem;

class OLEDBindings : public LuaBinding
{
public:
    explicit OLEDBindings(OLEDSystem *system);
    void registerAPI(lua_State *L) override;

private:
    OLEDSystem *oled;
    static OLEDSystem *get(lua_State *L);
    static int isSupported(lua_State *L);
    static int show(lua_State *L);
    static int clear(lua_State *L);
    static int currentMessage(lua_State *L);
};