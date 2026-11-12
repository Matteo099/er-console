#pragma once

#include "LuaBinding.h"

class TimeBindings : public LuaBinding {
  public:
    TimeBindings();
    void registerAPI(lua_State* L) override;
    static void setDelta(float dt);

  private:
    static int now(lua_State* L);
    static int seconds(lua_State* L);
    static int uptime(lua_State* L);
    static int clock(lua_State* L);
    static int delta(lua_State* L);

  private:
    static float s_delta;
};