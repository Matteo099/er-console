#pragma once

#include "LuaBinding.h"

class LuaConstantsBinding : public LuaBinding
{
public:
    LuaConstantsBinding() = default;
    void registerAPI(lua_State *L) override;
private:
    void registerPinMode(lua_State *L);
    void registerDigitalValue(lua_State *L);
    void registerTriggerMode(lua_State *L);
};