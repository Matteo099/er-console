#pragma once

#include "LuaBinding.h"

class InputSystem;

class InputBindings : public LuaBinding {
  public:
    explicit InputBindings(InputSystem* input);
    void registerAPI(lua_State* L) override;

  private:
    InputSystem* input;
};