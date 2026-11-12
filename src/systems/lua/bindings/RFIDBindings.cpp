#include "RFIDBindings.h"

#include "systems/input/subsystems/RFIDSystem.h"

RFIDBindings::RFIDBindings(RFIDSystem* system) : rfid(system) {
}

RFIDSystem* RFIDBindings::get(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, RFID_PTR_KEY);
    auto* system = static_cast<RFIDSystem*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return system;
}

/**
 *
 * local tag = input.rfid.consume()
 * if tag then
 *     print(tag.uid)
 * end
 */
// @doc Consume the next available RFID tag
// @return A table containing the tag information, or nil if no new tag is available
// @example local tag = input.rfid.consume()
// @lua input.rfid.consume() -> table|nil
int RFIDBindings::consume(lua_State* L) {
    auto* rfid = get(L);
    if (!rfid) {
        lua_pushnil(L);
        return 1;
    }
    RFIDTag tag;
    if (!rfid->consumeTag(tag)) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    lua_pushstring(L, tag.uid.c_str());
    lua_setfield(L, -2, "uid");
    lua_pushinteger(L, tag.timestampMs);
    lua_setfield(L, -2, "timestamp");
    return 1;
}

/**
 *
 * local tag = input.rfid.last()
 * if tag then
 *     print(tag.uid)
 * end
 */
// @doc Get the last available RFID tag
// @return A table containing the tag information, or nil if no tag is available
// @example local tag = input.rfid.last()
// @lua input.rfid.last() -> table|nil
int RFIDBindings::last(lua_State* L) {
    auto* rfid = get(L);
    if (!rfid) {
        lua_pushnil(L);
        return 1;
    }
    RFIDTag tag;
    if (!rfid->getLastTag(tag)) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    lua_pushstring(L, tag.uid.c_str());
    lua_setfield(L, -2, "uid");
    lua_pushinteger(L, tag.timestampMs);
    lua_setfield(L, -2, "timestamp");
    return 1;
}

void RFIDBindings::registerAPI(lua_State* L) {
    ensureTable(L, "input");
    lua_getglobal(L, "input");

    lua_newtable(L);
    lua_pushcfunction(L, consume);
    lua_setfield(L, -2, "consume");
    lua_pushcfunction(L, last);
    lua_setfield(L, -2, "last");
    lua_setfield(L, -2, "rfid");
    lua_pop(L, 1);
}