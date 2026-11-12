#pragma once

#include <vector>

#include "LuaBinding.h"

class LuaBindingManager {
  public:
    ~LuaBindingManager();

    void add(LuaBinding* binding);
    void registerAll(lua_State* L);
    void shutdownAll(lua_State* L);

  private:
    std::vector<LuaBinding*> bindings;
};