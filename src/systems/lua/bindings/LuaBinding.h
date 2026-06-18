#pragma once

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

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