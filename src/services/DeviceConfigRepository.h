#pragma once

#include <string>
#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"

struct DeviceWifiConfig
{
    std::string ssid;
    std::string password;
    bool enabled = true;
    bool autoConnect = true;
};

struct DeviceLocaleConfig
{
    std::string language = "it-IT";
    std::string timezone = "Europe/Rome";
    std::string dateFormat = "DD/MM/YYYY";
    double latitude = 0.0;
    double longitude = 0.0;
};

struct DevicePortalConfig
{
    std::string apSsid = "ER-Console-Setup";
    std::string apPassword;
    bool enabled = true;
};

struct DeviceUiConfig
{
    std::string theme = "dark";
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
    DevicePortalConfig portal;
    DeviceUiConfig ui;
    DeviceAudioConfig audio;
};

class DeviceConfigRepository
{
public:
    static constexpr const char* CONFIG_PATH = DEVICE_CONFIG_FILE;

    DeviceConfig defaultConfig() const;
    bool load(StorageSystem* storage, DeviceConfig& config) const;
    bool save(StorageSystem* storage, const DeviceConfig& config) const;
    std::string toJson(const DeviceConfig& config) const;

private:
    bool parseJson(const std::string& json, DeviceConfig& config) const;
};