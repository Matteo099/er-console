#pragma once

#include "LuaBinding.h"

class RFIDSystem;

class RFIDBindings : public LuaBinding {
  public:
    explicit RFIDBindings(RFIDSystem* system);
    void registerAPI(lua_State* L) override;

  private:
    RFIDSystem* rfid;

  private:
    static RFIDSystem* get(lua_State* L);
    static int consume(lua_State* L);
    static int last(lua_State* L);
};