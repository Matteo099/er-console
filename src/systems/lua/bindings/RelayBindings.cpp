#include "RelayBindings.h"

#include "systems/input/subsystems/RelaySystem.h"

RelaySystem* RelayBindings::getSystem(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, RELAY_PTR_KEY);
    auto* system = static_cast<RelaySystem*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return system;
}

// @doc Set the state of a relay
// @param relay The relay number (0-3)
// @param state The state to set (true for on, false for off)
// @example input.relays.set(0, true) -- turn on relay 0
// @lua input.relays.set(relay:int, state:bool) -> void
int RelayBindings::set(lua_State* L) {
    auto* system = getSystem(L);
    if (!system)
        return 0;
    uint8_t relay = luaL_checkinteger(L, 1);
    bool state = lua_toboolean(L, 2);
    system->set(relay, state);
    return 0;
}

// @doc Get the state of a relay
// @param relay The relay number (0-3)
// @return The state of the relay (true for on, false for off)
// @example local state = input.relays.get(0) -- get the state of relay 0
// @lua input.relays.get(relay:int) -> bool
int RelayBindings::get(lua_State* L) {
    auto* system = getSystem(L);
    if (!system) {
        lua_pushboolean(L, false);
        return 1;
    }
    uint8_t relay = luaL_checkinteger(L, 1);
    lua_pushboolean(L, system->get(relay));
    return 1;
}

// @doc Toggle the state of a relay
// @param relay The relay number (0-3)
// @example input.relays.toggle(0) -- toggle the state of relay 0
// @lua input.relays.toggle(relay:int) -> void
int RelayBindings::toggle(lua_State* L) {
    auto* system = getSystem(L);
    if (!system)
        return 0;
    uint8_t relay = luaL_checkinteger(L, 1);
    system->toggle(relay);
    return 0;
}

// @doc Pulse a relay for a specified duration
// @param relay The relay number (0-3)
// @param duration The duration of the pulse in milliseconds
// @example input.relays.pulse(0, 1000) -- pulse relay 0 for 1 second
// @lua input.relays.pulse(relay:int, duration:int) -> void
int RelayBindings::pulse(lua_State* L) {
    auto* system = getSystem(L);
    if (!system)
        return 0;
    uint8_t relay = luaL_checkinteger(L, 1);
    uint32_t duration = luaL_checkinteger(L, 2);
    system->pulse(relay, duration);
    return 0;
}

RelayBindings::RelayBindings(RelaySystem* system) : relays(system) {
}

void RelayBindings::registerAPI(lua_State* L) {
    //
    // store pointer
    //
    lua_pushlightuserdata(L, relays);
    lua_setfield(L, LUA_REGISTRYINDEX, RELAY_PTR_KEY);

    //
    // ensure input exists
    //
    ensureTable(L, "input");
    lua_getglobal(L, "input");

    //
    // create relay table
    //
    lua_newtable(L);
    lua_pushcfunction(L, set);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, get);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, toggle);
    lua_setfield(L, -2, "toggle");
    lua_pushcfunction(L, pulse);
    lua_setfield(L, -2, "pulse");
    lua_setfield(L, -2, "relays");
    lua_pop(L, 1);
}