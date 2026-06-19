#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define AUDIO_SYSTEM_PTR_KEY "audioSystem_ptr"
#define CHANNEL_PTR_KEY "lua.channelSystem"
#define OLED_PTR_KEY "lua.oledSystem"
#define RGB_PTR_KEY "lua.rgbSystem"
#define RELAY_PTR_KEY "lua.relaySystem"
#define RFID_PTR_KEY "lua.rfidSystem"
#define SEVEN_SEGMENT_PTR_KEY "lua.sevenSegmentSystem"

class LuaBinding
{
public:
    virtual ~LuaBinding() = default;

    virtual void registerAPI(lua_State *L) = 0;

protected:
    static void ensureTable(lua_State *L, const char *name)
    {
        lua_getglobal(L, name);
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_setglobal(L, name);
        }
        else
        {
            lua_pop(L, 1);
        }
    }
};