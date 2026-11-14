#pragma once

#include <cstddef>
#include <string>
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

struct DeviceConfig {
    std::string deviceName = "EscapeRoom Console";
    DeviceWifiConfig wifi;
    DeviceLocaleConfig locale;
    DeviceUiConfig ui;
    AudioPreferences audio;
};

class DeviceConfigRepository {
  public:
    static constexpr const char* NVS_NAMESPACE = "console-cfg";
    static constexpr const char* NVS_KEY = "settings";
    static constexpr size_t MAX_RECORD_BYTES = 2048;
    DeviceConfigRepository() = default;
    DeviceConfig defaultConfig() const;
    bool load(DeviceConfig& config) const;
    bool save(const DeviceConfig& config) const;
    bool savePreferences(const DeviceConfig& config) const;
    bool saveNetworkPreferences(const std::string& ssid, const std::string& password) const;
    static bool validNetwork(const std::string& ssid, const std::string& password);
    static bool validConfig(const DeviceConfig& config);
    static uint32_t revision();
    // Saved passwords are included only for internal NVS serialization.
    std::string toJson(const DeviceConfig& config, bool includeSecrets = false) const;

  private:
    enum class RecordState { Missing, Ready, Error };

    explicit DeviceConfigRepository(const char* target) : storageNamespace(target) {
    }

    bool parseJson(const std::string& json, DeviceConfig& config) const;
    RecordState readRecord(DeviceConfig& config) const;
    bool writeRecord(const DeviceConfig& config) const;
    const char* storageNamespace = NVS_NAMESPACE;
    friend void runConfigStorageDiagnostics();
};
