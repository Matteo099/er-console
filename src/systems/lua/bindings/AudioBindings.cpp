#include "AudioBindings.h"

#include "systems/audio/AudioSystem.h"

AudioBindings::AudioBindings(AudioSystem *system) : audio(system) {}

AudioSystem *AudioBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, "audioSystem_ptr");
    auto *system = static_cast<AudioSystem *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return system;
}

/**
 * audio.beep(1000, 250)
 */
int AudioBindings::beep(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;
    uint16_t frequency = static_cast<uint16_t>(luaL_optinteger(L, 1, 1000));
    uint16_t duration = static_cast<uint16_t>(luaL_optinteger(L, 2, 100));
    audio->beep(frequency, duration);
    return 0;
}

void AudioBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);
    lua_pushcfunction(L, beep);
    lua_setfield(L, -2, "beep");
    lua_setglobal(L, "audio");
}