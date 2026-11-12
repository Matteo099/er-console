#pragma once

#include <string>
#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"
#include "systems/audio/AudioPreferences.h"

struct DeviceWifiConfig {
    std::string ssid;
    std::string password;
};

struct DeviceLocaleConfig {
    std::string language = "it-IT";
    std::string timezone = "Europe/Rome";
    std::string dateFormat = "DD/MM/YYYY";
};

struct DeviceUiConfig {
    std::string theme = "mint";
    int brightness = 100;
};

struct DeviceAudioConfig : AudioPreferences {
    int volume = 70;
    bool enabled = true;
    bool independent = false;  // Legacy JSON/NVS migration only; not serialized.
};

struct DeviceConfig {
    std::string deviceName = "EscapeRoom Console";
    DeviceWifiConfig wifi;
    DeviceLocaleConfig locale;
    DeviceUiConfig ui;
    DeviceAudioConfig audio;
};

class DeviceConfigRepository {
  public:
    DeviceConfig defaultConfig() const;
    bool load(StorageSystem* storage, DeviceConfig& config) const;
    void loadCached(DeviceConfig& config) const;
    bool save(StorageSystem* storage, const DeviceConfig& config) const;
    bool saveUiPreferences(const std::string& theme, bool soundEnabled) const;
    bool savePreferences(const DeviceConfig& config) const;
    bool saveNetworkPreferences(const std::string& ssid, const std::string& password) const;
    static bool validNetwork(const std::string& ssid, const std::string& password);
    static uint32_t revision();
    std::string toJson(const DeviceConfig& config) const;
    // Stateless decoder also used by migration tests and configuration tooling.
    bool parseJson(const std::string& json, DeviceConfig& config) const;
};
