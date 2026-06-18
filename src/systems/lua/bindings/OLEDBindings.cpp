#include "OLEDBindings.h"

#include "systems/input/subsystems/OLEDSystem.h"

#define OLED_PTR_KEY "lua.oledSystem"

// -------------------------
// helper
// -------------------------
OLEDSystem *OLEDBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, OLED_PTR_KEY);
    auto *sys = static_cast<OLEDSystem *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return sys;
}

// -------------------------
// OLED: isSupported
// -------------------------
int OLEDBindings::isSupported(lua_State *L)
{
    auto *sys = get(L);
    lua_pushboolean(L, sys && sys->isSupported());
    return 1;
}

// -------------------------
// OLED: show(text)
// -------------------------
int OLEDBindings::show(lua_State *L)
{
    auto *sys = get(L);
    if (!sys)
        return 0;
    const char *text = luaL_checkstring(L, 1);
    sys->showMessage(text);
    return 0;
}

// -------------------------
// OLED: clear()
// -------------------------
int OLEDBindings::clear(lua_State *L)
{
    auto *sys = get(L);
    if (sys)
        sys->clear();
    return 0;
}

// -------------------------
// OLED: currentMessage()
// returns current buffer
// -------------------------
int OLEDBindings::currentMessage(lua_State *L)
{
    auto *sys = get(L);
    if (!sys)
    {
        lua_pushnil(L);
        return 1;
    }
    std::string msg = sys->currentMessage();
    lua_pushstring(L, msg.c_str());
    return 1;
}

// -------------------------
// constructor
// -------------------------
OLEDBindings::OLEDBindings(OLEDSystem *system) : oled(system) {}

// -------------------------
// register API
// -------------------------
void OLEDBindings::registerAPI(lua_State *L)
{
    // store pointer
    lua_pushlightuserdata(L, oled);
    lua_setfield(L, LUA_REGISTRYINDEX, OLED_PTR_KEY);

    // ensure input table exists
    ensureTable(L, "input");
    lua_getglobal(L, "input");

    // create oled table
    lua_newtable(L);
    lua_pushcfunction(L, isSupported);
    lua_setfield(L, -2, "isSupported");
    lua_pushcfunction(L, show);
    lua_setfield(L, -2, "show");
    lua_pushcfunction(L, clear);
    lua_setfield(L, -2, "clear");
    lua_pushcfunction(L, currentMessage);
    lua_setfield(L, -2, "currentMessage");

    // attach to input
    lua_setfield(L, -2, "oled");
    lua_pop(L, 1);
}