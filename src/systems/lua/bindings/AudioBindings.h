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
    static int play(lua_State *L);
    static int stop(lua_State *L);
    static int setVolume(lua_State *L);
    static int getVolume(lua_State *L);
    static int setSourceVolume(lua_State *L);
    static int getSourceVolume(lua_State *L);
    static int isPlaying(lua_State *L);
    static int preload(lua_State *L);
};
