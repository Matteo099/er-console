#pragma once

#include <vector>

#include "LuaBinding.h"

class LuaBindingManager
{

public:
    void add(LuaBinding *binding);

    void registerAll(lua_State *L);

private:
    std::vector<LuaBinding *> bindings;
};