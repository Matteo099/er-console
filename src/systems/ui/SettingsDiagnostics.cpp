#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED
#include "SettingsDiagnostics.h"
#include <Arduino.h>
#include <vector>
#include <cstring>
#include "systems/audio/AudioMixer.h"
#include "services/DeviceConfigRepository.h"
#include "localization/LocalizedText.h"
#include "components/Label.h"
#include "drivers/display/TouchInputState.h"
#include "services/GameMetadata.h"
#include "systems/lua/bindings/CoreBindings.h"
#include "systems/network/NetworkSystem.h"
namespace
{
    std::vector<uint8_t> wav(int16_t sample, uint32_t rate = 22050)
    {
        std::vector<uint8_t> result(52, 0);
        std::memcpy(result.data(), "RIFF", 4); std::memcpy(result.data()+8,"WAVEfmt ",8);
        result[4] = 44; result[16] = 16; result[20] = 1; result[22] = 1;
        for (unsigned i=0;i<4;++i) result[24+i] = uint8_t(rate >> (i*8));
        for (unsigned i=0;i<4;++i) result[28+i] = uint8_t((rate*2) >> (i*8));
        result[32] = 2; result[34] = 16;
        std::memcpy(result.data()+36,"data",4); result[40] = 8;
        for (unsigned i=44;i<52;i+=2) { result[i] = uint8_t(sample); result[i+1] = uint8_t(uint16_t(sample)>>8); }
        return result;
    }
    int16_t next(AudioMixer &mixer)
    {
        uint8_t bytes[2]{};
        if (mixer.read(bytes,2)!=2) return 0;
        return int16_t(uint16_t(bytes[0]) | uint16_t(bytes[1])<<8);
    }
}
void runSettingsDiagnostics()
{
    unsigned passed = 0, failed = 0;
    auto check = [&](bool value, const char *name) { if (value) ++passed; else ++failed; Serial.printf("@CHECK %s %s\n", value ? "PASS" : "FAIL", name); };
    check(WifiScan::signalBars(-40)==4 && WifiScan::signalBars(-65)==3 && WifiScan::signalBars(-75)==2 && WifiScan::signalBars(-90)==1 && WifiScan::signalBars(0)==0, "Wi-Fi RSSI signal levels");
    std::vector<WifiScanEntry> networks = {{"Lab",-90,true,"wpa2"},{"",-20,false,"open"},{"Guest",-75,false,"open"},{"Lab",-42,true,"wpa2"},{"Lab",-60,false,"open"}};
    WifiScan::normalize(networks);
    check(networks.size()==3 && networks[0].ssid=="Lab" && networks[0].signal==-42 && networks[1].ssid=="Lab" && !networks[1].secure && networks[2].ssid=="Guest", "Wi-Fi strongest duplicates, hidden SSIDs and security");
    networks.clear(); for (unsigned i=0;i<40;++i) networks.push_back({std::to_string(i),-40-int(i),true,"wpa2"});
    WifiScan::normalize(networks); check(networks.size()==24 && networks.back().signal==-63, "Wi-Fi results bounded to 24 strongest networks");
#if !WIFI_ENABLED
    NetworkSystem offline;
    check(!offline.requestScan() && offline.getScanSnapshot().state==WifiScanState::Disabled, "Wi-Fi build flag prevents scan requests");
#endif
    TouchInputState touch;
    auto point = touch.update({0, 0, false});
    check(!point.pressed && point.x == 0 && point.y == 0, "touch starts released");
    touch.update({95, 160, true});
    point = touch.update({205, 161, true});
    check(point.pressed && point.x == 205 && point.y == 161, "touch tracks drag coordinates");
    point = touch.update({0, 0, false});
    check(!point.pressed && point.x == 205 && point.y == 161, "release retains final drag position");
    auto music = wav(1000), effects = wav(2000);
    AudioMixer mixer(22050); AudioPreferences preferences;
    preferences.music = {true,50}; preferences.effects = {true,100}; mixer.configure(preferences);
    check(mixer.play(music.data(),music.size(),AudioChannel::Music,true) && mixer.play(effects.data(),effects.size(),AudioChannel::Effects,false), "PCM voices accepted");
    check(next(mixer)==2500, "independent gain and simultaneous mixing");
    mixer.stop(AudioChannel::Effects); check(next(mixer)==500, "stopping effects preserves music");
    preferences.effects.enabled=false; mixer.configure(preferences); check(next(mixer)==500, "muting effects preserves music");
    preferences.music.enabled=false; mixer.configure(preferences); check(!mixer.available(), "both muted generate no audio");
    preferences.music.enabled=true; mixer.configure(preferences); check(next(mixer)==500, "music resumes after mute");
    auto loud = wav(30000); preferences.music={true,100}; preferences.effects={true,100}; mixer.configure(preferences);
    mixer.play(loud.data(),loud.size(),AudioChannel::Music,true); mixer.play(loud.data(),loud.size(),AudioChannel::Effects,true);
    check(next(mixer)==32767, "mix clips safely without overflow");
    auto slow = wav(1500,11025); mixer.stop(); mixer.play(slow.data(),slow.size(),AudioChannel::Music,false);
    uint8_t output[32]{}; check(mixer.read(output,sizeof(output))==16, "sample rate conversion preserves duration");
    auto invalid=music; invalid[34]=8; check(!mixer.play(invalid.data(),invalid.size(),AudioChannel::Effects,false), "unsupported PCM format rejected");
    check(!mixer.play(music.data(),music.size()-1,AudioChannel::Effects,false), "truncated WAV rejected");
    mixer.stop(); check(!mixer.available(), "stop all voices");
    DeviceConfigRepository repository; DeviceConfig config;
    check(repository.parseJson("{\"audio\":{\"volume\":80,\"enabled\":false}}",config) && !config.audio.music.enabled && !config.audio.effects.enabled && config.audio.music.volume==80 && config.audio.effects.volume==80, "legacy audio migration");
    check(repository.parseJson("{\"audio\":{\"music\":{\"volume\":101,\"enabled\":false},\"effects\":{\"volume\":-1}},\"locale\":{\"language\":\"en-US\"}}",config) && !config.audio.music.enabled && config.audio.music.volume==100 && config.audio.effects.volume==0 && config.locale.language=="en-GB", "independent audio and language decode");
    check(repository.parseJson("{}",config) && config.locale.language=="it-IT" && config.audio.music.volume==50 && config.audio.effects.volume==70, "Italian defaults");
    check(DeviceConfigRepository::validNetwork(std::string(32,'s'),"password") && !DeviceConfigRepository::validNetwork(std::string(33,'s'),"password") && !DeviceConfigRepository::validNetwork("ssid","short"), "Wi-Fi credential boundaries");
    check(DeviceConfigRepository::validNetwork("ssid","") && DeviceConfigRepository::validNetwork("ssid",std::string(64,'a')) && !DeviceConfigRepository::validNetwork("ssid",std::string(64,'z')), "open networks and hex PSK validation");
    JsonDocument document;
    deserializeJson(document, "{\"name\":\"Legacy\",\"description\":\"Original\"}");
    GameInfo game{}; game.id = "legacy";
    GameMetadata::read(document.as<JsonObjectConst>(), game);
    check(game.titleFor(false) == "Legacy" && game.titleFor(true) == "Legacy", "legacy game metadata fallback");
    deserializeJson(document, "{\"name\":\"Legacy\",\"translations\":{\"it\":{\"name\":\"Mappa\",\"description\":\"Indizi\"},\"en\":{\"name\":\"Map\",\"description\":\"Clues\"}}}");
    GameMetadata::read(document.as<JsonObjectConst>(), game);
    check(game.titleFor(false) == "Mappa" && game.descriptionFor(true) == "Clues" && game.titleFor(true) == "Map", "bilingual game metadata retained in cache");
    const auto language = I18n::language();
    auto *vm = luaL_newstate();
    CoreBindings().registerAPI(vm);
    I18n::select(I18n::Language::Italian);
    check(luaL_dostring(vm, "return system.translate({it='Mappa',en='Map'})") == LUA_OK && std::strcmp(lua_tostring(vm,-1), "Mappa") == 0, "Lua game text Italian");
    lua_settop(vm,0);
    I18n::select(I18n::Language::English);
    check(luaL_dostring(vm, "return system.translate({it='Mappa',en='Map'})") == LUA_OK && std::strcmp(lua_tostring(vm,-1), "Map") == 0, "Lua game text English");
    lua_settop(vm,0);
    check(luaL_dostring(vm, "return system.translate({it='Mappa'})") == LUA_OK && std::strcmp(lua_tostring(vm,-1), "Mappa") == 0, "Lua missing translation fallback");
    lua_close(vm);
    I18n::select(I18n::Language::Italian);
    auto *root=lv_obj_create(nullptr); auto *label=Label::create(root,tr(Text::Settings));
    I18n::select(I18n::Language::English); LocalizedText::refresh();
    check(!std::strcmp(lv_label_get_text(label),"Settings"), "live label translation");
    lv_obj_delete(root); LocalizedText::refresh();
    bool complete=true;
    for (unsigned i=1;i<unsigned(Text::Count);++i) complete &= *tr(static_cast<Text>(i)) != 0;
    check(complete,"English catalog complete");
    I18n::select(I18n::Language::Italian); complete=true;
    for (unsigned i=1;i<unsigned(Text::Count);++i) complete &= *tr(static_cast<Text>(i)) != 0;
    check(complete,"Italian catalog complete");
    I18n::select(language); LocalizedText::refresh();
    Serial.printf("@SELFTEST passed=%u failed=%u\n",passed,failed);
}
#endif
