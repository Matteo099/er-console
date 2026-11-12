#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "AudioDiagnostics.h"
#include <Arduino.h>
#include <vector>
#include <cstring>
#include "systems/audio/AudioSystem.h"
#include "systems/lua/bindings/AudioBindings.h"

namespace {
void u32(std::vector<uint8_t>& data, size_t at, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i)
        data[at + i] = uint8_t(value >> (i * 8));
}

std::vector<uint8_t> wav(std::initializer_list<int16_t> samples, uint32_t rate = 22050,
                         unsigned channels = 1) {
    std::vector<uint8_t> data(44 + samples.size() * 2, 0);
    std::memcpy(data.data(), "RIFF", 4);
    std::memcpy(data.data() + 8, "WAVEfmt ", 8);
    u32(data, 4, data.size() - 8);
    u32(data, 16, 16);
    data[20] = 1;
    data[22] = channels;
    u32(data, 24, rate);
    u32(data, 28, rate * channels * 2);
    data[32] = channels * 2;
    data[34] = 16;
    std::memcpy(data.data() + 36, "data", 4);
    u32(data, 40, samples.size() * 2);
    size_t at = 44;
    for (auto sample : samples) {
        data[at++] = uint8_t(sample);
        data[at++] = uint8_t(uint16_t(sample) >> 8);
    }
    return data;
}

int16_t next(AudioMixer& mixer) {
    uint8_t data[2]{};
    if (mixer.read(data, 2) != 2)
        return 0;
    return int16_t(uint16_t(data[0]) | uint16_t(data[1]) << 8);
}
}  // namespace

void runAudioDiagnostics() {
    unsigned passed = 0, failed = 0;
    auto check = [&](bool value, const char* name) {
        value ? ++passed : ++failed;
        Serial.printf("@AUDIOCHECK %s %s\n", value ? "PASS" : "FAIL", name);
    };
    AudioMixer mixer(22050);
    AudioPreferences prefs;
    prefs.music = {true, 100};
    prefs.effects = {true, 100};
    mixer.configure(prefs);
    const auto a = wav({1000}), b = wav({2000}), c = wav({3000});
    check(mixer.play(a.data(), a.size(), AudioChannel::Music, true, 101, 100) &&
              mixer.play(b.data(), b.size(), AudioChannel::Music, true, 102, 50) &&
              mixer.play(c.data(), c.size(), AudioChannel::Effects, true, 103, 25),
          "three files and two music sources coexist");
    check(next(mixer) == 2750, "independent source gains mix exactly");
    check(mixer.setSourceVolume(101, 0) && next(mixer) == 1750,
          "changing one gain preserves other sources");
    check(mixer.stopSource(102) && next(mixer) == 750 && mixer.isPlaying(101) &&
              mixer.isPlaying(103),
          "stop targets a single source");
    check(!mixer.stopSource(102) && !mixer.setSourceVolume(999, 20),
          "stopped and invalid handles rejected");
    mixer.stop();
    const auto sequence = wav({1000, 2000});
    mixer.play(sequence.data(), sequence.size(), AudioChannel::Effects, true, 201, 100);
    check(next(mixer) == 1000, "first cursor starts at first frame");
    mixer.play(sequence.data(), sequence.size(), AudioChannel::Effects, true, 202, 50);
    check(next(mixer) == 2500 && next(mixer) == 2000,
          "shared WAV data has independent playback cursors");
    check(mixer.references(sequence.data()), "active shared data stays pinned");
    mixer.stopSource(201);
    check(mixer.references(sequence.data()), "one remaining voice keeps cache pinned");
    mixer.stopSource(202);
    check(!mixer.references(sequence.data()), "last voice releases cache pin");
    mixer.stop();
    const auto loud = wav({30000}), negative = wav({-30000});
    mixer.play(loud.data(), loud.size(), AudioChannel::Music, true);
    mixer.play(loud.data(), loud.size(), AudioChannel::Music, true);
    check(next(mixer) == 32767, "positive sum saturates without overflow");
    mixer.stop();
    mixer.play(negative.data(), negative.size(), AudioChannel::Effects, true);
    mixer.play(negative.data(), negative.size(), AudioChannel::Effects, true);
    check(next(mixer) == -32768, "negative sum saturates without overflow");
    mixer.stop();
    bool full = true;
    for (unsigned i = 0; i < AUDIO_MAX_SOURCES; ++i)
        full &= mixer.play(a.data(), a.size(), AudioChannel::Effects, true, 300 + i) != 0;
    check(full && next(mixer) == 8000, "eight simultaneous voices render");
    check(!mixer.play(b.data(), b.size(), AudioChannel::Effects, true, 399) && next(mixer) == 8000,
          "ninth voice does not steal an active source");
    mixer.stopSource(303);
    check(mixer.play(b.data(), b.size(), AudioChannel::Effects, true, 400) && next(mixer) == 9000,
          "freed voice slot is reusable");
    mixer.stop();
    const auto loop = wav({1000, -2000, 3000, -4000});
    mixer.play(loop.data(), loop.size(), AudioChannel::Music, true, 500);
    bool seamless = true;
    const int expected[] = {1000, -2000, 3000, -4000};
    for (unsigned i = 0; i < 4099; ++i)
        seamless &= next(mixer) == expected[i % 4];
    check(seamless && mixer.isPlaying(500), "indefinite loop wraps across thousands of boundaries");
    mixer.stopSource(500);
    check(!mixer.available(), "explicit stop ends a loop");
    mixer.play(loop.data(), loop.size(), AudioChannel::Effects, false, 501);
    uint8_t bytes[32]{};
    check(mixer.read(bytes, sizeof(bytes)) == 8 && !mixer.isPlaying(501) && !mixer.read(bytes, 2),
          "one-shot EOF is frame exact");
    mixer.play(loop.data(), loop.size(), AudioChannel::Effects, false, 502, 0);
    std::memset(bytes, 0x7f, sizeof(bytes));
    const int silent = mixer.read(bytes, 8);
    bool zero = silent == 8;
    for (unsigned i = 0; i < 8; ++i)
        zero &= bytes[i] == 0;
    check(zero && !mixer.isPlaying(502), "zero source gain advances silently to EOF");
    prefs.music = {true, 50};
    prefs.effects = {true, 25};
    mixer.configure(prefs);
    mixer.play(a.data(), a.size(), AudioChannel::Music, true, 510, 50);
    mixer.play(a.data(), a.size(), AudioChannel::Effects, true, 511, 50);
    check(next(mixer) == 375, "source and Settings bus gains multiply independently");
    prefs.music.enabled = false;
    prefs.effects.enabled = false;
    mixer.configure(prefs);
    check(!mixer.available() && mixer.isPlaying(510) && mixer.isPlaying(511),
          "bus mute preserves looping sources");
    prefs.effects.enabled = true;
    mixer.configure(prefs);
    check(next(mixer) == 125, "effects loop resumes without enabling music");
    mixer.stop(AudioChannel::Effects);
    check(mixer.isPlaying(510) && !mixer.isPlaying(511), "group stop preserves the other group");
    mixer.stop();
    prefs.music = {true, 100};
    prefs.effects = {true, 100};
    mixer.configure(prefs);
    const auto slow = wav({1000, 2000}, 11025);
    mixer.play(slow.data(), slow.size(), AudioChannel::Music, false);
    check(next(mixer) == 1000 && next(mixer) == 1000 && next(mixer) == 2000 &&
              next(mixer) == 2000 && !mixer.available(),
          "sample rate conversion preserves duration");
    const auto stereo = wav({1000, 3000, -1000, -3000}, 22050, 2);
    mixer.play(stereo.data(), stereo.size(), AudioChannel::Music, false);
    check(next(mixer) == 2000 && next(mixer) == -2000, "stereo is averaged to mono");
    auto junk = a;
    const uint8_t chunk[] = {'J', 'U', 'N', 'K', 1, 0, 0, 0, 42, 0};
    junk.insert(junk.begin() + 36, chunk, chunk + 10);
    u32(junk, 4, junk.size() - 8);
    check(mixer.play(junk.data(), junk.size(), AudioChannel::Music, false) && next(mixer) == 1000,
          "odd-sized metadata chunks and padding supported");
    bool malformed = true;
    for (unsigned variant = 0; variant < 6; ++variant) {
        auto invalid = a;
        if (variant == 0)
            invalid.pop_back();
        if (variant == 1)
            invalid[20] = 3;
        if (variant == 2)
            invalid[32] = 4;
        if (variant == 3)
            invalid[28] = 0;
        if (variant == 4)
            u32(invalid, 40, 0xffffffff);
        if (variant == 5)
            invalid[34] = 8;
        malformed &= !mixer.play(invalid.data(), invalid.size(), AudioChannel::Effects, false);
    }
    check(malformed, "truncated, float, bad alignment/rate and malformed WAVs rejected");
    mixer.play(a.data(), a.size(), AudioChannel::Music, true, 600);
    auto invalid = a;
    invalid[20] = 3;
    check(!mixer.play(invalid.data(), invalid.size(), AudioChannel::Music, true) &&
              mixer.isPlaying(600) && next(mixer) == 1000,
          "bad file cannot replace an existing voice");
    check(mixer.tone(1000, 1, 601) && next(mixer) == 1000 && mixer.isPlaying(600),
          "beep mixes without replacing WAV playback");
    mixer.stopSource(601);
    check(next(mixer) == 1000, "stopping beep preserves WAV");
    mixer.stop();
    mixer.play(a.data(), a.size(), AudioChannel::Effects, true);
    std::memset(bytes, 0xa5, sizeof(bytes));
    check(mixer.read(bytes, 19) == 18 && bytes[18] == 0xa5,
          "odd output buffer cannot split a PCM frame");
    AudioMixer invalidRate(0);
    check(!invalidRate.play(a.data(), a.size(), AudioChannel::Music, true) &&
              !invalidRate.tone(1000, 1),
          "invalid output rate cannot divide by zero");

    AudioSources pool;
    const auto old = pool.reserve(AudioChannel::Music, 25);
    pool.release(old);
    const auto replacement = pool.reserve(AudioChannel::Music, 80);
    check(old != replacement && !pool.release(old) && pool.find(replacement)->volume == 80,
          "generation handles protect reused slots");
    auto queue = xQueueCreate(2, sizeof(AudioEvent));
    auto* audio = new AudioSystem(queue);  // No init/update: no driver I/O or SD access.
    const auto first = audio->play("first.wav", true, AudioChannel::Music, 20);
    const auto second = audio->play("second.wav", true, AudioChannel::Music, 70);
    check(first && second && first != second && audio->isPlaying(first) && audio->isPlaying(second),
          "control queue accepts multiple sources in one group");
    uint8_t level = 0;
    check(audio->setSourceVolume(first, 35) && audio->getSourceVolume(first, level) &&
              level == 35 && uxQueueMessagesWaiting(queue) == 2,
          "source volume mailbox works while event queue is full");
    check(audio->stopSource(first) && !audio->isPlaying(first) && audio->isPlaying(second),
          "source stop is not lost behind full queue");
    check(!audio->play("third.wav", false, AudioChannel::Effects),
          "full command queue is reported to caller");
    xQueueReset(queue);
    const auto third = audio->play("third.wav", false, AudioChannel::Effects);
    check(third && !audio->stopSource(first) && audio->isPlaying(third),
          "stale stop cannot kill replacement command");
    audio->stop(AudioChannel::Music);
    check(!audio->isPlaying(second) && audio->isPlaying(third),
          "queued group stop preserves another group");
    audio->stop();
    check(!audio->isPlaying(third), "stop all clears queued sources");
    delete audio;
    vQueueDelete(queue);

    queue = xQueueCreate(16, sizeof(AudioEvent));
    audio = new AudioSystem(queue);
    auto* vm = luaL_newstate();
    luaL_openlibs(vm);
    lua_pushlightuserdata(vm, audio);
    lua_setfield(vm, LUA_REGISTRYINDEX, AUDIO_SYSTEM_PTR_KEY);
    AudioBindings(audio).registerAPI(vm);
    const char* script = R"lua(
        local a=assert(audio.play('a.wav',{loop=true,channel='music',volume=25}))
        local b=assert(audio.play('b.wav',true,'music',70))
        assert(a~=b and audio.isPlaying(a) and audio.isPlaying(b))
        assert(audio.setSourceVolume(a,40) and audio.getSourceVolume(a)==40)
        assert(audio.getSourceVolume(b)==70 and audio.getVolume()==100)
        assert(audio.stop(a) and not audio.stop(a) and audio.isPlaying(b))
        assert(audio.stop('music') and not audio.isPlaying(b))
        assert(not pcall(audio.play,'bad.wav',{volume=101}))
        assert(not pcall(audio.play,'bad.wav',{channel='invalid'}))
        assert(audio.preload('a.wav'))
        audio.setVolume(25); assert(audio.getVolume()==25)
    )lua";
    const bool luaOk = luaL_dostring(vm, script) == LUA_OK;
    check(luaOk, "Lua options/legacy API, handles, source volume and master compatibility");
    if (!luaOk)
        Serial.printf("@AUDIOERROR %s\n", lua_tostring(vm, -1));
    lua_close(vm);
    delete audio;
    vQueueDelete(queue);
    Serial.printf("@AUDIOTEST passed=%u failed=%u\n", passed, failed);
}
#endif
