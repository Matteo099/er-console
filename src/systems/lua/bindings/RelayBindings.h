#pragma once

#include "LuaBinding.h"

class RelaySystem;

class RelayBindings : public LuaBinding {
  public:
    explicit RelayBindings(RelaySystem* system);
    void registerAPI(lua_State* L) override;

  private:
    RelaySystem* relays;
    static RelaySystem* getSystem(lua_State* L);
    static int set(lua_State* L);
    static int get(lua_State* L);
    static int toggle(lua_State* L);
    static int pulse(lua_State* L);
};