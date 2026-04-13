#include "DeviceConfigRepository.h"

#include <Arduino.h>
#include <ArduinoJson.h>

DeviceConfig DeviceConfigRepository::defaultConfig() const
{
    return DeviceConfig{};
}

bool DeviceConfigRepository::load(StorageSystem* storage, DeviceConfig& config) const
{
    if (!storage || !storage->isReady())
        return false;

    std::string json;
    if (!storage->readTextFile(CONFIG_PATH, json))
    {
        config = defaultConfig();
        return false;
    }

    return parseJson(json, config);
}

bool DeviceConfigRepository::save(StorageSystem* storage, const DeviceConfig& config) const
{
    if (!storage || !storage->isReady())
        return false;

    return storage->writeTextFile(CONFIG_PATH, toJson(config), true);
}

std::string DeviceConfigRepository::toJson(const DeviceConfig& config) const
{
    JsonDocument doc;
    doc["version"] = 1;
    doc["deviceName"] = config.deviceName;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = config.wifi.ssid;
    wifi["password"] = config.wifi.password;
    wifi["enabled"] = config.wifi.enabled;
    wifi["autoConnect"] = config.wifi.autoConnect;

    JsonObject locale = doc["locale"].to<JsonObject>();
    locale["language"] = config.locale.language;
    locale["timezone"] = config.locale.timezone;
    locale["dateFormat"] = config.locale.dateFormat;
    locale["latitude"] = config.locale.latitude;
    locale["longitude"] = config.locale.longitude;

    JsonObject portal = doc["portal"].to<JsonObject>();
    portal["apSsid"] = config.portal.apSsid;
    portal["apPassword"] = config.portal.apPassword;
    portal["enabled"] = config.portal.enabled;

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
        config.wifi.enabled = wifi["enabled"] | true;
        config.wifi.autoConnect = wifi["autoConnect"] | true;
    }

    JsonObject locale = doc["locale"];
    if (!locale.isNull())
    {
        config.locale.language = locale["language"].as<std::string>();
        config.locale.timezone = locale["timezone"].as<std::string>();
        config.locale.dateFormat = locale["dateFormat"].as<std::string>();
        config.locale.latitude = locale["latitude"] | 0.0;
        config.locale.longitude = locale["longitude"] | 0.0;
    }

    JsonObject portal = doc["portal"];
    if (!portal.isNull())
    {
        config.portal.apSsid = portal["apSsid"].as<std::string>();
        config.portal.apPassword = portal["apPassword"].as<std::string>();
        config.portal.enabled = portal["enabled"] | true;
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

    if (config.portal.apSsid.empty())
        config.portal.apSsid = defaultConfig().portal.apSsid;

    return true;
}