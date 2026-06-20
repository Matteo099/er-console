#include "ChannelBindings.h"

#include "systems/input/subsystems/ChannelSystem.h"

/*
    Helper:
    Retrieve ChannelSystem pointer
*/
ChannelSystem *ChannelBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, CHANNEL_PTR_KEY);
    auto *channel = static_cast<ChannelSystem *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return channel;
}

/**
 * inputs.channels.pinMode(OUTPUT, 5)
 */
int ChannelBindings::pinMode(lua_State *L)
{
    auto *channels = get(L);
    if (!channels)
        return 0;
    uint8_t mode = luaL_checkinteger(L, 1);
    uint16_t channel = luaL_checkinteger(L, 2);
    channels->pinMode(channel, mode);
    return 0;
}

/**
 * inputs.channels.digitalWrite(5, true)
 */
int ChannelBindings::digitalWrite(lua_State *L)
{
    auto *channels = get(L);
    if (!channels)
        return 0;
    uint16_t channel = luaL_checkinteger(L, 1);
    bool value = lua_toboolean(L, 2);
    channels->digitalWrite(channel, value);
    return 0;
}

/**
 * local value = inputs.channels.digitalRead(10)
 */
int ChannelBindings::digitalRead(lua_State *L)
{
    auto *channels = get(L);
    if (!channels)
    {
        lua_pushboolean(L, false);
        return 1;
    }
    uint16_t channel = luaL_checkinteger(L, 1);
    bool value = channels->digitalRead(channel);
    lua_pushboolean(L, value);
    return 1;
}

/**
 * local value = inputs.channels.analogRead(200)
 */
int ChannelBindings::analogRead(lua_State *L)
{
    auto *channels = get(L);
    if (!channels)
    {
        lua_pushinteger(L, 0);
        return 1;
    }
    uint16_t channel = luaL_checkinteger(L, 1);
    int value = channels->analogRead(channel);
    lua_pushinteger(L, value);
    return 1;
}

/**
 * inputs.channels.analogWrite(200, 128)
 */
int ChannelBindings::analogWrite(lua_State *L)
{
    auto *channels = get(L);
    if (!channels)
        return 0;
    uint16_t channel = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);
    channels->analogWrite(channel, value);
    return 0;
}

ChannelBindings::ChannelBindings(ChannelSystem *system) : channels(system) {}

void ChannelBindings::registerAPI(lua_State *L)
{
    // store pointer
    lua_pushlightuserdata(L, channels);
    lua_setfield(L, LUA_REGISTRYINDEX, CHANNEL_PTR_KEY);

    // create: input.channels
    ensureTable(L, "input");
    lua_getglobal(L, "input");

    lua_newtable(L);
    lua_pushcfunction(L, pinMode);
    lua_setfield(L, -2, "pinMode");
    lua_pushcfunction(L, digitalWrite);
    lua_setfield(L, -2, "digitalWrite");
    lua_pushcfunction(L, digitalRead);
    lua_setfield(L, -2, "digitalRead");
    lua_pushcfunction(L, analogRead);
    lua_setfield(L, -2, "analogRead");
    lua_pushcfunction(L, analogWrite);
    lua_setfield(L, -2, "analogWrite");
    lua_setfield(L, -2, "channels");
    lua_pop(L, 1);
}