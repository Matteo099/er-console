#include "SevenSegmentBindings.h"

#include "systems/input/subsystems/SevenSegmentSystem.h"

#define SEVEN_SEGMENT_PTR_KEY "lua.sevenSegmentSystem"

SevenSegmentSystem *SevenSegmentBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, SEVEN_SEGMENT_PTR_KEY);
    auto *system = static_cast<SevenSegmentSystem *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return system;
}

/**
 * input.sevenSegment.display( 0, "1234" )
 */
int SevenSegmentBindings::display(lua_State *L)
{
    auto *system = get(L);
    if (!system)
        return 0;
    uint8_t index = luaL_checkinteger(L, 1);
    const char *value = luaL_checkstring(L, 2);
    system->display(index, value);
    return 0;
}

/**
 * input.sevenSegment.clear(0)
 */
int SevenSegmentBindings::clear(lua_State *L)
{
    auto *system = get(L);
    if (!system)
        return 0;
    uint8_t index = luaL_checkinteger(L, 1);
    system->clear(index);
    return 0;
}

/**
 * input.sevenSegment.brightness(0, 5)
 */
int SevenSegmentBindings::brightness(lua_State *L)
{
    auto *system = get(L);
    if (!system)
        return 0;
    uint8_t index = luaL_checkinteger(L, 1);
    uint8_t value = luaL_checkinteger(L, 2);
    system->brightness(index, value);
    return 0;
}

SevenSegmentBindings::SevenSegmentBindings(SevenSegmentSystem *system) : sevenSegment(system) {}

void SevenSegmentBindings::registerAPI(lua_State *L)
{
    //
    // store pointer
    //
    lua_pushlightuserdata(L, sevenSegment);
    lua_setfield(L, LUA_REGISTRYINDEX, SEVEN_SEGMENT_PTR_KEY);

    //
    // ensure input table exists
    //
    ensureTable(L, "input");
    lua_getglobal(L, "input");

    //
    // create input.sevenSegment
    //
    lua_newtable(L);
    lua_pushcfunction(L, display);
    lua_setfield(L, -2, "display");
    lua_pushcfunction(L, clear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, brightness);
    lua_setfield(L, -2, "brightness");
    lua_setfield(L, -2, "sevenSegment");
    lua_pop(L, 1);
}