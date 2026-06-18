#include "CoreBindings.h"

#include "log/Logger.h"

/**
 * Lua API for logging and system info:
 * system.info("Game started")
 * system.warn("Low power")
 * system.error("RFID reader unavailable")
 * system.debug("Player position: x=10, y=20")
 * local logs = system.logs(10) -> returns recent logs as a string
 * system.clearLogs() -> clears recent logs buffer
 */

int CoreBindings::info(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::info(Log::LUA_TAG, "%s", msg);
    return 0;
}

int CoreBindings::warn(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::warn(Log::LUA_TAG, "%s", msg);
    return 0;
}

int CoreBindings::error(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::error(Log::LUA_TAG, "%s", msg);
    return 0;
}

int CoreBindings::debug(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::debug(Log::LUA_TAG, "%s", msg);
    return 0;
}

int CoreBindings::logs(lua_State *L)
{
    size_t maxLines = luaL_optinteger(L, 1, 200);
    std::string content = Log::getRecentLogs(maxLines);
    lua_pushstring(L, content.c_str());
    return 1;
}

int CoreBindings::clearLogs(lua_State *L)
{
    Log::clearRecentLogs();
    return 0;
}

void CoreBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);
    lua_pushcfunction(L, info);
    lua_setfield(L, -2, "info");
    lua_pushcfunction(L, warn);
    lua_setfield(L, -2, "warn");
    lua_pushcfunction(L, error);
    lua_setfield(L, -2, "error");
    lua_pushcfunction(L, debug);
    lua_setfield(L, -2, "debug");
    lua_pushcfunction(L, logs);
    lua_setfield(L, -2, "logs");
    lua_pushcfunction(L, clearLogs);
    lua_setfield(L, -2, "clearLogs");
    lua_setglobal(L, "system");
}