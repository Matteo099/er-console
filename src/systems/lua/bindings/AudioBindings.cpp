#include "AudioBindings.h"

#include "systems/audio/AudioSystem.h"
#include <cstring>
#include <limits>

namespace
{
    AudioSourceId sourceId(lua_State *L, int argument)
    {
        const auto id = luaL_checkinteger(L,argument);
        luaL_argcheck(L,id > 0 && uint64_t(id) <= std::numeric_limits<AudioSourceId>::max(),argument,"invalid audio source id");
        return AudioSourceId(id);
    }
    uint8_t sourceLevel(lua_State *L, int argument)
    {
        const auto level = luaL_checkinteger(L,argument);
        luaL_argcheck(L,level >= 0 && level <= 100,argument,"volume must be 0..100");
        return uint8_t(level);
    }
}

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
// @param loop Optional. Boolean or {loop, channel, volume} options (default: false).
// @param channel Optional. "music" or "effects"; loops default to music, one-shots to effects.
// @example audio.play("beep.wav")
// @example audio.play("music.wav", true)
// @param volume Optional per-source gain, 0-100 (default 100).
// @example local id = audio.play("music.wav", {loop=true, channel="music", volume=40})
// @return Accepted source id, or nil and an error message. Loading happens asynchronously.
// @lua audio.play(path:string, loop:boolean|AudioPlayOptions=false, channel:string=nil, volume:uint8=100) -> integer|nil
int AudioBindings::play(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;

    const char *path = luaL_checkstring(L, 1);
    bool loop = false;
    const char *channel = nullptr;
    uint8_t level = 100;
    if (lua_istable(L,2))
    {
        lua_getfield(L,2,"loop");
        if (!lua_isnil(L,-1)) luaL_checktype(L,-1,LUA_TBOOLEAN);
        loop = lua_toboolean(L,-1); lua_pop(L,1);
        lua_getfield(L,2,"channel"); channel = luaL_optstring(L,-1,loop ? "music" : "effects"); lua_pop(L,1);
        lua_getfield(L,2,"volume"); if (!lua_isnil(L,-1)) level = sourceLevel(L,lua_gettop(L)); lua_pop(L,1);
    }
    else
    {
        if (!lua_isnoneornil(L,2)) luaL_checktype(L,2,LUA_TBOOLEAN);
        loop = lua_toboolean(L,2);
        channel = luaL_optstring(L,3,loop ? "music" : "effects");
        if (!lua_isnoneornil(L,4)) level = sourceLevel(L,4);
    }
    if (std::strcmp(channel, "music") && std::strcmp(channel, "effects")) return luaL_error(L, "channel must be music or effects");
    const auto id = audio->play(path,loop,!std::strcmp(channel,"music") ? AudioChannel::Music : AudioChannel::Effects,level);
    if (!id) { lua_pushnil(L); lua_pushliteral(L,"no free audio source, queue full, or invalid path"); return 2; }
    lua_pushinteger(L,id); return 1;
}

// @doc Stop a source id, music, effects, or both groups (default).
// @param channel Optional. Source id, "music", "effects" or "all".
// @example audio.stop()
// @lua audio.stop(channel:string|integer="all") -> boolean
int AudioBindings::stop(lua_State *L)
{
    auto *audio = get(L);
    if (!audio)
        return 0;
    if (lua_type(L,1) == LUA_TNUMBER) { lua_pushboolean(L,audio->stopSource(sourceId(L,1))); return 1; }
    const char *channel = luaL_optstring(L, 1, "all");
    if (std::strcmp(channel, "music") && std::strcmp(channel, "effects") && std::strcmp(channel, "all")) return luaL_error(L, "channel must be music, effects or all");
    audio->stop(!std::strcmp(channel, "music") ? AudioChannel::Music : !std::strcmp(channel, "effects") ? AudioChannel::Effects : AudioChannel::All);
    lua_pushboolean(L,true); return 1;
}

// @doc Change one source's gain; does not change Settings or other sources.
// @lua audio.setSourceVolume(id:integer, volume:uint8) -> boolean
int AudioBindings::setSourceVolume(lua_State *L)
{
    auto *audio = get(L); const auto id = sourceId(L,1); const auto level = sourceLevel(L,2);
    lua_pushboolean(L,audio && audio->setSourceVolume(id,level)); return 1;
}
// @lua audio.getSourceVolume(id:integer) -> integer|nil
int AudioBindings::getSourceVolume(lua_State *L)
{
    auto *audio = get(L); uint8_t level;
    if (audio && audio->getSourceVolume(sourceId(L,1),level)) lua_pushinteger(L,level); else lua_pushnil(L);
    return 1;
}
// @doc True while a source is queued or playing; false after EOF, failure or stop.
// @lua audio.isPlaying(id:integer) -> boolean
int AudioBindings::isPlaying(lua_State *L)
{
    auto *audio = get(L); const auto id = sourceId(L,1);
    lua_pushboolean(L,audio && audio->isPlaying(id)); return 1;
}
// @doc Queue a cache load before starting playback, to avoid cold SD reads later.
// @lua audio.preload(path:string) -> boolean
int AudioBindings::preload(lua_State *L)
{
    auto *audio = get(L); const char *path = luaL_checkstring(L,1);
    lua_pushboolean(L,audio && audio->preload(path)); return 1;
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
    lua_pushcfunction(L,setSourceVolume); lua_setfield(L,-2,"setSourceVolume");
    lua_pushcfunction(L,getSourceVolume); lua_setfield(L,-2,"getSourceVolume");
    lua_pushcfunction(L,isPlaying); lua_setfield(L,-2,"isPlaying");
    lua_pushcfunction(L,preload); lua_setfield(L,-2,"preload");

    lua_setglobal(L, "audio");
}
