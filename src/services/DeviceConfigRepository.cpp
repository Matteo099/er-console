#include "DeviceConfigRepository.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <nvs.h>

namespace
{
    void loadUiPreferences(DeviceConfig &config)
    {
        nvs_handle_t handle;
        if (nvs_open("console-ui", NVS_READONLY, &handle) != ESP_OK) return;
        char theme[32];
        size_t length = sizeof(theme);
        if (nvs_get_str(handle, "theme", theme, &length) == ESP_OK) config.ui.theme = theme;
        uint8_t sound = config.audio.enabled ? 1 : 0;
        if (nvs_get_u8(handle, "sound", &sound) == ESP_OK) config.audio.enabled = sound != 0;
        nvs_close(handle);
    }
}

DeviceConfig DeviceConfigRepository::defaultConfig() const
{
    return DeviceConfig{};
}

bool DeviceConfigRepository::load(StorageSystem* storage, DeviceConfig& config) const
{
    config = defaultConfig();
    bool loaded = false;
    if (storage && storage->isReady())
    {
        std::string json;
        if (storage->readTextFile(DEVICE_CONFIG_FILE, json)) loaded = parseJson(json, config);
    }
    // Local display preferences remain usable without SD. All callers, including
    // the portal, see the same effective UI configuration through this repository.
    loadUiPreferences(config);
    return loaded;
}

bool DeviceConfigRepository::save(StorageSystem* storage, const DeviceConfig& config) const
{
    if (!storage || !storage->isReady())
        return false;

    if (!storage->writeTextFile(DEVICE_CONFIG_FILE, toJson(config), true)) return false;
    return saveUiPreferences(config.ui.theme, config.audio.enabled);
}

bool DeviceConfigRepository::saveUiPreferences(const std::string &theme, bool soundEnabled) const
{
    nvs_handle_t handle;
    if (nvs_open("console-ui", NVS_READWRITE, &handle) != ESP_OK) return false;
    esp_err_t result = nvs_set_str(handle, "theme", theme.c_str());
    if (result == ESP_OK) result = nvs_set_u8(handle, "sound", soundEnabled ? 1 : 0);
    if (result == ESP_OK) result = nvs_commit(handle);
    nvs_close(handle);
    return result == ESP_OK;
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
    ui["brightness"] = config.ui.brightness;

    JsonObject audio = doc["audio"].to<JsonObject>();
    audio["volume"] = config.audio.volume;
    audio["enabled"] = config.audio.enabled;

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
        config.locale.language = locale["language"].as<std::string>();
        config.locale.timezone = locale["timezone"].as<std::string>();
        config.locale.dateFormat = locale["dateFormat"].as<std::string>();
    }

    JsonObject ui = doc["ui"];
    if (!ui.isNull())
    {
        config.ui.theme = ui["theme"].as<std::string>();
        config.ui.brightness = ui["brightness"] | 100;
    }

    JsonObject audio = doc["audio"];
    if (!audio.isNull())
    {
        config.audio.volume = audio["volume"] | 70;
        config.audio.enabled = audio["enabled"] | true;
    }

    if (config.deviceName.empty())
        config.deviceName = defaultConfig().deviceName;

    return true;
}
