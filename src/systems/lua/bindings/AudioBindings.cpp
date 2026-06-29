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

// @doc Play a beep.
// @param frequency Tone frequency in Hz.
// @param duration Duration in milliseconds.
// @example audio.beep(1000, 250)
// @lua audio.beep(frequency:uint16=1000, duration:uint16=100) -> void
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

// @doc Play a wav audio file from storage
// @param path Path to the audio file in storage
// @param loop Optional. True to loop playback (default: false)
// @example audio.play("beep.wav")
// @example audio.play("music.wav", true)
// @lua audio.play(path:string, loop:boolean=false) -> void
int AudioBindings::play(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;

    const char *path = luaL_checkstring(L, 1);
    bool loop = lua_toboolean(L, 2); // defaults to false if omitted or nil
    audio->play(path, loop);
    return 0;
}

// @doc Stop the currently playing audio
// @example audio.stop()
// @lua audio.stop() -> void
int AudioBindings::stop(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;
    audio->stop();
    return 0;
}

// @doc Set the audio volume
// @param volume The volume level (0-100)
// @example audio.setVolume(80)
// @lua audio.setVolume(volume:uint8) -> void
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

// @doc Get the current audio volume
// @example currentVolume = audio.getVolume()
// @return Current volume value (0-100)
// @lua audio.getVolume() -> uint8
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