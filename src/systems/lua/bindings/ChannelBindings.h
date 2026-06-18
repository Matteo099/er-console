#pragma once

#include "LuaBinding.h"

class ChannelSystem;

class ChannelBindings : public LuaBinding
{
public:
    explicit ChannelBindings(ChannelSystem *channels);
    void registerAPI(lua_State *L) override;

private:
    ChannelSystem *channels;

    static ChannelSystem *get(lua_State *L);
    static int pinMode(lua_State *L);
    static int digitalWrite(lua_State *L);
    static int digitalRead(lua_State *L);
    static int analogRead(lua_State *L);
    static int analogWrite(lua_State *L);
};