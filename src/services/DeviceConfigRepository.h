#pragma once

#include <string>
#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"

struct DeviceWifiConfig
{
    std::string ssid;
    std::string password;
};

struct DeviceLocaleConfig
{
    std::string language = "it-IT";
    std::string timezone = "Europe/Rome";
    std::string dateFormat = "DD/MM/YYYY";
};

struct DeviceUiConfig
{
    std::string theme = "mint";
    int brightness = 100;
};

struct DeviceAudioConfig
{
    int volume = 70;
    bool enabled = true;
};

struct DeviceConfig
{
    std::string deviceName = "EscapeRoom Console";
    DeviceWifiConfig wifi;
    DeviceLocaleConfig locale;
    DeviceUiConfig ui;
    DeviceAudioConfig audio;
};

class DeviceConfigRepository
{
public:
    DeviceConfig defaultConfig() const;
    bool load(StorageSystem* storage, DeviceConfig& config) const;
    bool save(StorageSystem* storage, const DeviceConfig& config) const;
    bool saveUiPreferences(const std::string &theme, bool soundEnabled) const;
    std::string toJson(const DeviceConfig& config) const;

private:
    bool parseJson(const std::string& json, DeviceConfig& config) const;
};
