#pragma once

#include "LuaBinding.h"

class AudioSystem;
class AudioBindings : public LuaBinding
{
public:
    explicit AudioBindings(AudioSystem *system);
    void registerAPI(lua_State *L) override;

private:
    AudioSystem *audio;

private:
    static AudioSystem *get(lua_State *L);
    static int beep(lua_State *L);
};