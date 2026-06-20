#include "LuaBindingManager.h"

LuaBindingManager::~LuaBindingManager()
{
    for (auto* b : bindings)
    {
        delete b;
    }
}

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