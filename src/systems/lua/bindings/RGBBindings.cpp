#include "RGBBindings.h"

#include "systems/input/subsystems/RgbLedSystem.h"

RgbLedSystem *RGBBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, RGB_PTR_KEY);
    auto *system = static_cast<RgbLedSystem *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return system;
}

// @doc Set the color of a specific RGB LED
// @param led The LED number (0-3)
// @param r The red component (0-255)
// @param g The green component (0-255)
// @param b The blue component (0-255)
// @example input.rgb.setColor(0, 255, 0, 0) -- sets LED 0 to red
// @lua input.rgb.setColor(led:int, r:int, g:int, b:int) -> void
int RGBBindings::setColor(lua_State *L)
{
    auto *system = get(L);
    if (!system)
        return 0;
    uint8_t led = luaL_checkinteger(L, 1);
    uint8_t r = luaL_checkinteger(L, 2);
    uint8_t g = luaL_checkinteger(L, 3);
    uint8_t b = luaL_checkinteger(L, 4);
    LedColor color;
    color.r = r;
    color.g = g;
    color.b = b;
    system->setColor(led, color);
    return 0;
}

// @doc Set the brightness of a specific RGB LED
// @param led The LED number (0-3)
// @param value The brightness value (0-255)
// @example input.rgb.setBrightness(0, 128) -- sets LED 0 to half brightness
// @lua input.rgb.setBrightness(led:int, value:int) -> void
int RGBBindings::setBrightness(lua_State *L)
{
    auto *system = get(L);
    if (!system)
        return 0;
    uint8_t led = luaL_checkinteger(L, 1);
    uint8_t value = luaL_checkinteger(L, 2);
    system->setBrightness(led, value);
    return 0;
}

RGBBindings::RGBBindings(RgbLedSystem *system) : rgb(system) {}

void RGBBindings::registerAPI(lua_State *L)
{
    //
    // store pointer
    //
    lua_pushlightuserdata(L, rgb);
    lua_setfield(L, LUA_REGISTRYINDEX, RGB_PTR_KEY);

    //
    // ensure input exists
    //
    ensureTable(L, "input");
    lua_getglobal(L, "input");

    //
    // create rgb table
    //
    lua_newtable(L);
    lua_pushcfunction(L, setColor);
    lua_setfield(L, -2, "setColor");
    lua_pushcfunction(L, setBrightness);
    lua_setfield(L, -2, "setBrightness");

    //
    // input.rgb
    //
    lua_setfield(L, -2, "rgb");
    lua_pop(L, 1);
}