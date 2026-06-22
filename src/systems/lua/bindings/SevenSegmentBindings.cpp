#include "SevenSegmentBindings.h"

#include "systems/input/subsystems/SevenSegmentSystem.h"

SevenSegmentSystem *SevenSegmentBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, SEVEN_SEGMENT_PTR_KEY);
    auto *system = static_cast<SevenSegmentSystem *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return system;
}

// @doc Display a value on a specific seven-segment display
// @param display The display number (0-3)
// @param value The value to display (string)
// @example input.sevenSegment.display(0, "1234") -- displays "1234" on display 0
// @lua input.sevenSegment.display(display:int, value:string) -> void
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

// @doc Clear a specific seven-segment display
// @param display The display number (0-3)
// @example input.sevenSegment.clear(0) -- clears display 0
// @lua input.sevenSegment.clear(display:int) -> void
int SevenSegmentBindings::clear(lua_State *L)
{
    auto *system = get(L);
    if (!system)
        return 0;
    uint8_t index = luaL_checkinteger(L, 1);
    system->clear(index);
    return 0;
}

// @doc Set the brightness of a specific seven-segment display
// @param display The display number (0-3)
// @param value The brightness value (0-255)
// @example input.sevenSegment.brightness(0, 128) -- sets display 0 to half brightness
// @lua input.sevenSegment.brightness(display:int, value:int) -> void
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