#include "AudioBindings.h"

#include "systems/audio/AudioSystem.h"

AudioBindings::AudioBindings(AudioSystem *system) : audio(system) {}

AudioSystem *AudioBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, AUDIO_SYSTEM_PTR_KEY);
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

/**
 * audio.play("path/to/file.wav")
 */
int AudioBindings::play(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;
    const char *path = luaL_checkstring(L, 1);
    audio->play(path);
    return 0;
}

/**
 * audio.stop()
 */
int AudioBindings::stop(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;
    audio->stop();
    return 0;
}

/**
 * audio.setVolume(80)
 */
int AudioBindings::setVolume(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;
    int volume = static_cast<int>(luaL_checkinteger(L, 1));
    if (volume < 0)
        volume = 0;
    if (volume > 100)
        volume = 100;
    audio->setVolume(static_cast<uint8_t>(volume));
    return 0;
}

/**
 * local v = audio.getVolume()
 */
int AudioBindings::getVolume(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
    {
        lua_pushinteger(L, 0);
        return 1;
    }
    lua_pushinteger(L, audio->getVolume());
    return 1;
}

void AudioBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);

    lua_pushcfunction(L, beep);
    lua_setfield(L, -2, "beep");

    lua_pushcfunction(L, play);
    lua_setfield(L, -2, "play");

    lua_pushcfunction(L, stop);
    lua_setfield(L, -2, "stop");

    lua_pushcfunction(L, setVolume);
    lua_setfield(L, -2, "setVolume");

    lua_pushcfunction(L, getVolume);
    lua_setfield(L, -2, "getVolume");

    lua_setglobal(L, "audio");
}