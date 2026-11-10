#include "CoreBindings.h"
#include "systems/ui/localization/I18n.h"

#include "log/Logger.h"

// @doc Log an info message to the console
// @param msg The info message to log
// @example system.info("Hello from Lua!")
// @lua system.info(msg:string) -> void
int CoreBindings::info(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::info(Log::LUA_SCRIPT_TAG, "%s", msg);
    return 0;
}

// @doc Log a warning message to the console
// @param msg The warning message to log
// @example system.warn("Low power")
// @lua system.warn(msg:string) -> void
int CoreBindings::warn(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::warn(Log::LUA_SCRIPT_TAG, "%s", msg);
    return 0;
}

// @doc Log an error message to the console
// @param msg The error message to log
// @example system.error("RFID reader unavailable")
// @lua system.error(msg:string) -> void
int CoreBindings::error(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::error(Log::LUA_SCRIPT_TAG, "%s", msg);
    return 0;
}

// @doc Log a debug message to the console
// @param msg The debug message to log
// @example system.debug("Player position: x=10, y=20")
// @lua system.debug(msg:string) -> void
int CoreBindings::debug(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    Log::debug(Log::LUA_SCRIPT_TAG, "%s", msg);
    return 0;
}

// @doc Get recent logs as a string
// @param maxLines The maximum number of log lines to retrieve (default: 200)
// @return A string containing the recent logs
// @example system.logs(10)
// @lua system.logs(maxLines?:uint16) -> string
int CoreBindings::logs(lua_State *L)
{
    size_t maxLines = luaL_optinteger(L, 1, 200);
    std::string content = Log::getRecentLogs(maxLines);
    lua_pushstring(L, content.c_str());
    return 1;
}

// @doc Clear the recent logs buffer
// @example system.clearLogs()
// @lua system.clearLogs() -> void
int CoreBindings::clearLogs(lua_State *L)
{
    Log::clearRecentLogs();
    return 0;
}

void CoreBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);
    lua_pushcfunction(L, language);
    lua_setfield(L, -2, "language");
    lua_pushcfunction(L, translate);
    lua_setfield(L, -2, "translate");
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

int CoreBindings::language(lua_State *L) { lua_pushstring(L, I18n::code()); return 1; }

// @doc Resolve an authored game message using the console language, then Italian/English fallback.
// @example system.translate({it = "Mappa", en = "Map"})
// @lua system.translate(message:table) -> string
int CoreBindings::translate(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *keys[] = {I18n::language() == I18n::Language::English ? "en" : "it", "it", "en"};
    for (const char *key : keys)
    {
        lua_getfield(L, 1, key);
        if (lua_type(L, -1) == LUA_TSTRING) return 1;
        lua_pop(L, 1);
    }
    return luaL_error(L, "translation needs an it or en string");
}
