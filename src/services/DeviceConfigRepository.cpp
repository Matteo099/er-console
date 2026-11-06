#include "DeviceConfigRepository.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <nvs.h>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <atomic>
#include "core/TaskMutex.h"

namespace
{
    TaskMutex configMutex;
    DeviceConfig cachedConfig;
    bool hasConfig = false;
    std::atomic<uint32_t> preferencesRevision{0};
    struct PreferenceRecord
    {
        uint32_t magic = 0x45525032;
        char theme[12]{};
        char language[8]{};
        uint8_t musicEnabled, musicVolume, effectsEnabled, effectsVolume;
    };
    static_assert(sizeof(PreferenceRecord) == 28, "Stable NVS preferences layout");
    struct NetworkRecord { uint32_t magic = 0x45524e31; char ssid[33]{}; char password[65]{}; };
    const char *languageCode(const std::string &value) { return value == "en" || value.compare(0, 3, "en-") == 0 ? "en-GB" : "it-IT"; }
    bool validTheme(const std::string &value) { return value == "mint" || value == "sky" || value == "coral"; }
    void legacyAudio(DeviceAudioConfig &audio) { audio.music = audio.effects = {audio.enabled, uint8_t(std::max(0, std::min(100, audio.volume)))}; }
    void loadUiPreferences(DeviceConfig &config)
    {
        nvs_handle_t handle;
        if (nvs_open("console-ui", NVS_READONLY, &handle) == ESP_OK)
        {
            PreferenceRecord record{};
            size_t size = sizeof(record);
            const bool valid = nvs_get_blob(handle, "prefs-v2", &record, &size) == ESP_OK && size == sizeof(record) &&
                record.magic == 0x45525032 && record.theme[11] == 0 && record.language[7] == 0 &&
                validTheme(record.theme) && record.musicEnabled <= 1 && record.effectsEnabled <= 1 &&
                record.musicVolume <= 100 && record.effectsVolume <= 100 &&
                (std::strcmp(record.language,"it-IT") == 0 || std::strcmp(record.language,"en-GB") == 0);
            if (valid)
            {
                config.ui.theme = record.theme;
                config.locale.language = record.language;
                config.audio.music = {record.musicEnabled != 0, record.musicVolume};
                config.audio.effects = {record.effectsEnabled != 0, record.effectsVolume};
                config.audio.independent = true;
            }
            else
            {
                char theme[32]; size_t length = sizeof(theme);
                if (nvs_get_str(handle, "theme", theme, &length) == ESP_OK && validTheme(theme)) config.ui.theme = theme;
                uint8_t sound;
                if (nvs_get_u8(handle, "sound", &sound) == ESP_OK && (!config.audio.independent || !sound))
                    config.audio.music.enabled = config.audio.effects.enabled = sound != 0;
            }
            config.audio.enabled = config.audio.effects.enabled;
            config.audio.volume = config.audio.effects.volume;
            nvs_close(handle);
        }
        if (nvs_open("console-net", NVS_READONLY, &handle) == ESP_OK)
        {
            NetworkRecord record{}; size_t size = sizeof(record);
            if (nvs_get_blob(handle, "credentials", &record, &size) == ESP_OK && size == sizeof(record) &&
                record.magic == 0x45524e31 && !record.ssid[32] && !record.password[64] &&
                DeviceConfigRepository::validNetwork(record.ssid, record.password))
                config.wifi = {record.ssid, record.password};
            nvs_close(handle);
        }
    }

}

DeviceConfig DeviceConfigRepository::defaultConfig() const
{
    return DeviceConfig{};
}
uint32_t DeviceConfigRepository::revision() { return preferencesRevision.load(); }

bool DeviceConfigRepository::load(StorageSystem* storage, DeviceConfig& config) const
{
    config = defaultConfig();
    bool loaded = false;
    if (storage && storage->isReady())
    {
        std::string json;
        if (storage->readTextFile(DEVICE_CONFIG_FILE, json)) loaded = parseJson(json, config);
    }
    if (!loaded)
    {
        TaskMutex::Guard lock(configMutex);
        if (hasConfig) config = cachedConfig;
    }
    // Local display preferences remain usable without SD. All callers, including
    // the portal, see the same effective UI configuration through this repository.
    loadUiPreferences(config);
    { TaskMutex::Guard lock(configMutex); cachedConfig = config; hasConfig |= loaded; }
    return loaded;
}

void DeviceConfigRepository::loadCached(DeviceConfig &config) const
{
    TaskMutex::Guard lock(configMutex);
    config = cachedConfig;
    loadUiPreferences(config);
}

bool DeviceConfigRepository::save(StorageSystem* storage, const DeviceConfig& config) const
{
    if (!validTheme(config.ui.theme) || config.audio.music.volume > 100 || config.audio.effects.volume > 100 || !validNetwork(config.wifi.ssid, config.wifi.password)) return false;
    if (!storage || !storage->isReady())
        return false;

    if (!storage->writeTextFile(DEVICE_CONFIG_FILE, toJson(config), true)) return false;
    { TaskMutex::Guard lock(configMutex); cachedConfig = config; hasConfig = true; }
    return savePreferences(config) && saveNetworkPreferences(config.wifi.ssid, config.wifi.password);
}

bool DeviceConfigRepository::savePreferences(const DeviceConfig &config) const
{
    if (!validTheme(config.ui.theme) || config.audio.music.volume > 100 || config.audio.effects.volume > 100) return false;
    PreferenceRecord record{};
    std::strncpy(record.theme, config.ui.theme.c_str(), sizeof(record.theme)-1);
    std::strncpy(record.language, languageCode(config.locale.language), sizeof(record.language)-1);
    record.musicEnabled = config.audio.music.enabled; record.musicVolume = config.audio.music.volume;
    record.effectsEnabled = config.audio.effects.enabled; record.effectsVolume = config.audio.effects.volume;
    nvs_handle_t handle;
    if (nvs_open("console-ui", NVS_READWRITE, &handle) != ESP_OK) return false;
    const bool saved = nvs_set_blob(handle, "prefs-v2", &record, sizeof(record)) == ESP_OK && nvs_commit(handle) == ESP_OK;
    nvs_close(handle);
    if (saved)
    {
        TaskMutex::Guard lock(configMutex);
        cachedConfig.ui = config.ui; cachedConfig.locale = config.locale; cachedConfig.audio = config.audio;
        cachedConfig.audio.enabled = config.audio.effects.enabled; cachedConfig.audio.volume = config.audio.effects.volume;
        preferencesRevision.fetch_add(1);
    }
    return saved;
}

bool DeviceConfigRepository::saveUiPreferences(const std::string &theme, bool enabled) const
{
    DeviceConfig config; loadCached(config);
    config.ui.theme = theme;
    config.audio.music.enabled = config.audio.effects.enabled = enabled;
    return savePreferences(config);
}

bool DeviceConfigRepository::validNetwork(const std::string &ssid, const std::string &password)
{
    if (ssid.size() > 32 || password.size() > 64 || ssid.find('\0') != std::string::npos || password.find('\0') != std::string::npos) return false;
    if (password.size() == 64) return std::all_of(password.begin(), password.end(), [](unsigned char c) { return std::isxdigit(c); });
    return password.empty() || (password.size() >= 8 && password.size() <= 63);
}

bool DeviceConfigRepository::saveNetworkPreferences(const std::string &ssid, const std::string &password) const
{
    if (!validNetwork(ssid, password)) return false;
    NetworkRecord record{};
    std::memcpy(record.ssid, ssid.data(), ssid.size()); std::memcpy(record.password, password.data(), password.size());
    nvs_handle_t handle;
    if (nvs_open("console-net", NVS_READWRITE, &handle) != ESP_OK) return false;
    const bool saved = nvs_set_blob(handle, "credentials", &record, sizeof(record)) == ESP_OK && nvs_commit(handle) == ESP_OK;
    nvs_close(handle);
    if (saved) { TaskMutex::Guard lock(configMutex); cachedConfig.wifi = {ssid, password}; preferencesRevision.fetch_add(1); }
    return saved;
}

std::string DeviceConfigRepository::toJson(const DeviceConfig& config) const
{
    JsonDocument doc;
    doc["version"] = 1;
    doc["deviceName"] = config.deviceName;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = config.wifi.ssid;
    wifi["password"] = config.wifi.password;

    JsonObject locale = doc["locale"].to<JsonObject>();
    locale["language"] = config.locale.language;
    locale["timezone"] = config.locale.timezone;
    locale["dateFormat"] = config.locale.dateFormat;

    JsonObject ui = doc["ui"].to<JsonObject>();
    ui["theme"] = config.ui.theme;
    ui["compactMode"] = false;
    ui["brightness"] = config.ui.brightness;

    JsonObject audio = doc["audio"].to<JsonObject>();
    audio["enabled"] = config.audio.effects.enabled;
    audio["volume"] = config.audio.effects.volume;
    audio["music"]["enabled"] = config.audio.music.enabled;
    audio["music"]["volume"] = config.audio.music.volume;
    audio["effects"]["enabled"] = config.audio.effects.enabled;
    audio["effects"]["volume"] = config.audio.effects.volume;

    String output;
    serializeJsonPretty(doc, output);
    output += '\n';
    return std::string(output.c_str());
}

bool DeviceConfigRepository::parseJson(const std::string& json, DeviceConfig& config) const
{
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, json);
    if (error)
        return false;

    config = defaultConfig();

    config.deviceName = doc["deviceName"].as<std::string>();

    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull())
    {
        config.wifi.ssid = wifi["ssid"].as<std::string>();
        config.wifi.password = wifi["password"].as<std::string>();
    }

    JsonObject locale = doc["locale"];
    if (!locale.isNull())
    {
        config.locale.language = locale["language"] | config.locale.language;
        config.locale.timezone = locale["timezone"] | config.locale.timezone;
        config.locale.dateFormat = locale["dateFormat"] | config.locale.dateFormat;
    }

    JsonObject ui = doc["ui"];
    if (!ui.isNull())
    {
        config.ui.theme = ui["theme"] | config.ui.theme;
        config.ui.brightness = ui["brightness"] | 100;
    }

    JsonObject audio = doc["audio"];
    if (!audio.isNull())
    {
        config.audio.volume = audio["volume"] | 70;
        config.audio.enabled = audio["enabled"] | true;
        if (!audio["volume"].isNull() || !audio["enabled"].isNull()) legacyAudio(config.audio);
        if (auto music = audio["music"].as<JsonObject>())
        {
            config.audio.independent = true;
            config.audio.music.enabled = music["enabled"] | config.audio.music.enabled;
            config.audio.music.volume = std::max(0, std::min(100, music["volume"] | int(config.audio.music.volume)));
        }
        if (auto effects = audio["effects"].as<JsonObject>())
        {
            config.audio.independent = true;
            config.audio.effects.enabled = effects["enabled"] | config.audio.effects.enabled;
            config.audio.effects.volume = std::max(0, std::min(100, effects["volume"] | int(config.audio.effects.volume)));
        }
    }

    if (config.deviceName.empty())
        config.deviceName = defaultConfig().deviceName;

    config.locale.language = languageCode(config.locale.language);
    if (!validTheme(config.ui.theme))
        config.ui.theme = config.ui.theme == "blue" ? "sky" : config.ui.theme == "red" ? "coral" : "mint";
    return true;
}
