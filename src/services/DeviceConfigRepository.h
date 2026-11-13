#pragma once

#include <string>
#include <functional>
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
    static constexpr const char* NVS_NAMESPACE = "console-cfg";
    static constexpr const char* NVS_KEY = "settings";
    static constexpr size_t MAX_RECORD_BYTES = 2048;
    DeviceConfigRepository() = default;
    DeviceConfig defaultConfig() const;
    bool load(StorageSystem* storage, DeviceConfig& config) const;
    void loadCached(DeviceConfig& config) const;
    bool save(StorageSystem* storage, const DeviceConfig& config) const;
    bool saveUiPreferences(const std::string& theme, bool soundEnabled) const;
    bool savePreferences(const DeviceConfig& config) const;
    bool saveNetworkPreferences(const std::string& ssid, const std::string& password) const;
    static bool validNetwork(const std::string& ssid, const std::string& password);
    static uint32_t revision();
    // Public/API serialization never returns a saved Wi-Fi password.
    std::string toJson(const DeviceConfig& config, bool includeSecrets = false) const;
    // Stateless decoder also used by migration tests and configuration tooling.
    bool parseJson(const std::string& json, DeviceConfig& config) const;

  private:
    enum class RecordState { Missing, Ready, Error };

    struct LegacySource {
        bool available = false;
        std::function<RecordState(const char*, size_t, std::string&)> read;
        std::function<bool(const char*)> remove;
    };

    DeviceConfigRepository(const char* target, const char* ui, const char* network)
        : storageNamespace(target), oldUiNamespace(ui), oldNetworkNamespace(network) {
    }

    bool loadLegacy(const LegacySource& source, DeviceConfig& config) const;
    RecordState readRecord(DeviceConfig& config) const;
    bool writeRecord(const DeviceConfig& config) const;
    void remember(const DeviceConfig& config) const;
    static bool validConfig(const DeviceConfig& config);
    const char* storageNamespace = NVS_NAMESPACE;
    const char* oldUiNamespace = "console-ui";
    const char* oldNetworkNamespace = "console-net";
    friend void runConfigStorageDiagnostics();
};
