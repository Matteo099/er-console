#pragma once

#include "LuaBinding.h"

class CoreBindings : public LuaBinding
{
public:
    void registerAPI(lua_State *L) override;

private:
    static int info(lua_State *L);
    static int warn(lua_State *L);
    static int error(lua_State *L);
    static int debug(lua_State *L);
    static int logs(lua_State *L);
    static int clearLogs(lua_State *L);
    static int language(lua_State *L);
    static int translate(lua_State *L);
};
