#include "LuaBindingManager.h"

void LuaBindingManager::add(LuaBinding *binding)
{
    bindings.push_back(binding);
}

void LuaBindingManager::registerAll(lua_State *L)
{
    for (auto *b : bindings)
    {
        if (b)
            b->registerAPI(L);
    }
}