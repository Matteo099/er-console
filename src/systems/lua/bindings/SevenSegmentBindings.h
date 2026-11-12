#pragma once

#include "LuaBinding.h"

class SevenSegmentSystem;

class SevenSegmentBindings : public LuaBinding {
  public:
    explicit SevenSegmentBindings(SevenSegmentSystem* system);
    void registerAPI(lua_State* L) override;

  private:
    SevenSegmentSystem* sevenSegment;
    static SevenSegmentSystem* get(lua_State* L);
    static int display(lua_State* L);
    static int clear(lua_State* L);
    static int brightness(lua_State* L);
};