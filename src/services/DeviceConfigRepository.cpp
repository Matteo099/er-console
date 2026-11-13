#include "DeviceConfigRepository.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <nvs.h>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <atomic>
#include <memory>
#include <sstream>
#include "core/TaskMutex.h"
#include "log/Logger.h"

namespace {
TaskMutex configMutex;
DeviceConfig cachedConfig;
bool hasConfig = false;
std::atomic<uint32_t> preferencesRevision{0};

struct PreferenceRecord {
    uint32_t magic = 0x45525032;
    char theme[12]{};
    char language[8]{};
    uint8_t musicEnabled, musicVolume, effectsEnabled, effectsVolume;
};

static_assert(sizeof(PreferenceRecord) == 28, "Stable NVS preferences layout");

struct NetworkRecord {
    uint32_t magic = 0x45524e31;
    char ssid[33]{};
    char password[65]{};
};

const char* languageCode(const std::string& value) {
    return value == "en" || value.compare(0, 3, "en-") == 0 ? "en-GB" : "it-IT";
}

bool validTheme(const std::string& value) {
    return value == "mint" || value == "sky" || value == "coral";
}

void legacyAudio(DeviceAudioConfig& audio) {
    audio.music =
        audio.effects = {audio.enabled, uint8_t(std::max(0, std::min(100, audio.volume)))};
}

bool loadOldPreferences(DeviceConfig& config, const char* uiNamespace,
                        const char* networkNamespace) {
    bool loaded = false;
    nvs_handle_t handle;
    if (nvs_open(uiNamespace, NVS_READONLY, &handle) == ESP_OK) {
        PreferenceRecord record{};
        size_t size = sizeof(record);
        const bool valid =
            nvs_get_blob(handle, "prefs-v2", &record, &size) == ESP_OK && size == sizeof(record) &&
            record.magic == 0x45525032 && record.theme[11] == 0 && record.language[7] == 0 &&
            validTheme(record.theme) && record.musicEnabled <= 1 && record.effectsEnabled <= 1 &&
            record.musicVolume <= 100 && record.effectsVolume <= 100 &&
            (std::strcmp(record.language, "it-IT") == 0 ||
             std::strcmp(record.language, "en-GB") == 0);
        if (valid) {
            loaded = true;
            config.ui.theme = record.theme;
            config.locale.language = record.language;
            config.audio.music = {record.musicEnabled != 0, record.musicVolume};
            config.audio.effects = {record.effectsEnabled != 0, record.effectsVolume};
            config.audio.independent = true;
        } else {
            char theme[32];
            size_t length = sizeof(theme);
            if (nvs_get_str(handle, "theme", theme, &length) == ESP_OK && validTheme(theme)) {
                config.ui.theme = theme;
                loaded = true;
            }
            uint8_t sound;
            if (nvs_get_u8(handle, "sound", &sound) == ESP_OK && sound <= 1 &&
                (!config.audio.independent || !sound)) {
                config.audio.music.enabled = config.audio.effects.enabled = sound != 0;
                loaded = true;
            }
        }
        config.audio.enabled = config.audio.effects.enabled;
        config.audio.volume = config.audio.effects.volume;
        nvs_close(handle);
    }
    if (nvs_open(networkNamespace, NVS_READONLY, &handle) == ESP_OK) {
        NetworkRecord record{};
        size_t size = sizeof(record);
        if (nvs_get_blob(handle, "credentials", &record, &size) == ESP_OK &&
            size == sizeof(record) && record.magic == 0x45524e31 && !record.ssid[32] &&
            !record.password[64] &&
            DeviceConfigRepository::validNetwork(record.ssid, record.password)) {
            config.wifi = {record.ssid, record.password};
            loaded = true;
        }
        nvs_close(handle);
    }
    return loaded;
}

}  // namespace

DeviceConfig DeviceConfigRepository::defaultConfig() const {
    return DeviceConfig{};
}

uint32_t DeviceConfigRepository::revision() {
    return preferencesRevision.load();
}

bool DeviceConfigRepository::load(StorageSystem* storage, DeviceConfig& config) const {
    LegacySource source;
    source.available = storage && storage->isReady();
    if (source.available) {
        source.read = [storage](const char* path, size_t limit, std::string& text) {
            if (!storage->exists(path))
                return RecordState::Missing;
            std::unique_ptr<IFile> file(storage->open(path));
            if (!file || file->length() > limit)
                return RecordState::Error;
            text.resize(file->length());
            size_t offset = 0;
            while (offset < text.size()) {
                const int count =
                    file->read(reinterpret_cast<uint8_t*>(&text[offset]), text.size() - offset);
                if (count <= 0 || size_t(count) > text.size() - offset) {
                    file->close();
                    return RecordState::Error;
                }
                offset += count;
            }
            file->close();
            return RecordState::Ready;
        };
        source.remove = [storage](const char* path) {
            if (!storage->exists(path))
                return true;
            auto file = storage->openNative(path);
            if (!file || file.isDirectory())
                return false;
            file.close();
            return storage->removePath(path);
        };
    }
    return loadLegacy(source, config);
}

void DeviceConfigRepository::loadCached(DeviceConfig& config) const {
    load(nullptr, config);
}

bool DeviceConfigRepository::save(StorageSystem* storage, const DeviceConfig& config) const {
    TaskMutex::Guard lock(configMutex);
    if (!writeRecord(config))
        return false;
    // Also retry cleanup after an explicit save/reset. SD is never a write target.
    DeviceConfig saved;
    load(storage, saved);
    return true;
}

bool DeviceConfigRepository::savePreferences(const DeviceConfig& config) const {
    TaskMutex::Guard lock(configMutex);
    DeviceConfig current;
    if (readRecord(current) == RecordState::Error)
        return false;
    loadCached(current);
    current.ui.theme = config.ui.theme;
    current.locale.language = config.locale.language;
    current.audio = config.audio;
    return writeRecord(current);
}

bool DeviceConfigRepository::saveUiPreferences(const std::string& theme, bool enabled) const {
    DeviceConfig config;
    loadCached(config);
    config.ui.theme = theme;
    config.audio.music.enabled = config.audio.effects.enabled = enabled;
    return savePreferences(config);
}

bool DeviceConfigRepository::validNetwork(const std::string& ssid, const std::string& password) {
    if (ssid.size() > 32 || password.size() > 64 || ssid.find('\0') != std::string::npos ||
        password.find('\0') != std::string::npos)
        return false;
    if (password.size() == 64)
        return std::all_of(password.begin(), password.end(), [](unsigned char c) {
            return std::isxdigit(c);
        });
    return password.empty() || (password.size() >= 8 && password.size() <= 63);
}

bool DeviceConfigRepository::saveNetworkPreferences(const std::string& ssid,
                                                    const std::string& password) const {
    if (!validNetwork(ssid, password))
        return false;
    TaskMutex::Guard lock(configMutex);
    DeviceConfig config;
    if (readRecord(config) == RecordState::Error)
        return false;
    loadCached(config);
    config.wifi = {ssid, password};
    return writeRecord(config);
}

void DeviceConfigRepository::remember(const DeviceConfig& config) const {
    if (!std::strcmp(storageNamespace, NVS_NAMESPACE)) {
        cachedConfig = config;
        hasConfig = true;
    }
}

bool DeviceConfigRepository::validConfig(const DeviceConfig& config) {
    const auto text = [](const std::string& value, size_t maximum) {
        return !value.empty() && value.size() <= maximum && value.find('\0') == std::string::npos;
    };
    return text(config.deviceName, 64) && text(config.locale.timezone, 64) &&
           text(config.locale.dateFormat, 32) && validTheme(config.ui.theme) &&
           config.ui.brightness >= 0 && config.ui.brightness <= 100 &&
           config.audio.music.volume <= 100 && config.audio.effects.volume <= 100 &&
           (config.locale.language == "it" || config.locale.language == "it-IT" ||
            config.locale.language == "en" || config.locale.language == "en-GB" ||
            config.locale.language == "en-US") &&
           validNetwork(config.wifi.ssid, config.wifi.password) &&
           (!config.wifi.ssid.empty() || config.wifi.password.empty());
}

DeviceConfigRepository::RecordState DeviceConfigRepository::readRecord(DeviceConfig& config) const {
    nvs_handle_t handle;
    auto result = nvs_open(storageNamespace, NVS_READONLY, &handle);
    if (result == ESP_ERR_NVS_NOT_FOUND)
        return RecordState::Missing;
    if (result != ESP_OK)
        return RecordState::Error;
    size_t size = 0;
    result = nvs_get_blob(handle, NVS_KEY, nullptr, &size);
    if (result == ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return RecordState::Missing;
    }
    std::string json;
    if (result == ESP_OK && size && size <= MAX_RECORD_BYTES) {
        json.resize(size);
        result = nvs_get_blob(handle, NVS_KEY, &json[0], &size);
    } else
        result = ESP_FAIL;
    nvs_close(handle);
    JsonDocument doc;
    if (result != ESP_OK || deserializeJson(doc, json) || !doc["version"].is<unsigned>() ||
        doc["version"].as<unsigned>() != 2 || !doc["deviceName"].is<const char*>() ||
        !doc["wifi"]["ssid"].is<const char*>() || !doc["wifi"]["password"].is<const char*>() ||
        !doc["locale"]["language"].is<const char*>() ||
        !doc["locale"]["timezone"].is<const char*>() ||
        !doc["locale"]["dateFormat"].is<const char*>() || !doc["ui"]["theme"].is<const char*>() ||
        !doc["ui"]["brightness"].is<int>())
        return RecordState::Error;
    for (const char* channel : {"music", "effects"})
        if (!doc["audio"][channel]["enabled"].is<bool>() ||
            !doc["audio"][channel]["volume"].is<unsigned>() ||
            doc["audio"][channel]["volume"].as<unsigned>() > 100)
            return RecordState::Error;
    // Validate before the legacy decoder normalizes aliases or clamps values.
    DeviceConfig decoded;
    decoded.deviceName = doc["deviceName"].as<std::string>();
    decoded.wifi = {doc["wifi"]["ssid"].as<std::string>(),
                    doc["wifi"]["password"].as<std::string>()};
    decoded.locale.language = doc["locale"]["language"].as<std::string>();
    decoded.locale.timezone = doc["locale"]["timezone"].as<std::string>();
    decoded.locale.dateFormat = doc["locale"]["dateFormat"].as<std::string>();
    decoded.ui.theme = doc["ui"]["theme"].as<std::string>();
    decoded.ui.brightness = doc["ui"]["brightness"].as<int>();
    if (!validConfig(decoded) || !parseJson(json, decoded))
        return RecordState::Error;
    config = std::move(decoded);
    return RecordState::Ready;
}

bool DeviceConfigRepository::writeRecord(const DeviceConfig& value) const {
    if (!validConfig(value)) {
        Log::warn(Log::STORAGE_TAG,
                  "Invalid device settings: name_bytes=%u timezone_bytes=%u date_bytes=%u "
                  "brightness=%d network_valid=%u",
                  unsigned(value.deviceName.size()), unsigned(value.locale.timezone.size()),
                  unsigned(value.locale.dateFormat.size()), value.ui.brightness,
                  validNetwork(value.wifi.ssid, value.wifi.password));
        return false;
    }
    DeviceConfig config = value;
    config.locale.language = languageCode(config.locale.language);
    config.audio.enabled = config.audio.effects.enabled;
    config.audio.volume = config.audio.effects.volume;
    config.audio.independent = true;
    const auto json = toJson(config, true);
    if (json.size() > MAX_RECORD_BYTES)
        return false;
    DeviceConfig previous;
    if (readRecord(previous) == RecordState::Ready && toJson(previous, true) == json) {
        remember(config);
        return true;
    }
    nvs_handle_t handle;
    const auto opened = nvs_open(storageNamespace, NVS_READWRITE, &handle);
    if (opened != ESP_OK) {
        Log::error(Log::STORAGE_TAG, "NVS settings open failed: %s", esp_err_to_name(opened));
        return false;
    }
    auto result = nvs_set_blob(handle, NVS_KEY, json.data(), json.size());
    if (result == ESP_OK)
        result = nvs_commit(handle);
    nvs_close(handle);
    DeviceConfig verified;
    if (result != ESP_OK || readRecord(verified) != RecordState::Ready ||
        toJson(verified, true) != json) {
        Log::error(Log::STORAGE_TAG, "NVS settings save/verification failed: %s",
                   esp_err_to_name(result));
        return false;
    }
    remember(verified);
    if (!std::strcmp(storageNamespace, NVS_NAMESPACE))
        preferencesRevision.fetch_add(1);
    return true;
}

bool DeviceConfigRepository::loadLegacy(const LegacySource& source, DeviceConfig& config) const {
    TaskMutex::Guard lock(configMutex);
    const auto state = readRecord(config);
    if (state != RecordState::Ready) {
        config = hasConfig && !std::strcmp(storageNamespace, NVS_NAMESPACE) ? cachedConfig
                                                                            : defaultConfig();
        if (state == RecordState::Error) {
            Log::error(Log::STORAGE_TAG,
                       "Device NVS configuration unreadable; SD migration skipped");
            return false;
        }
        bool valid = true;
        if (source.available) {
            std::string json, wifi;
            const auto jsonState = source.read(DEVICE_CONFIG_FILE, MAX_RECORD_BYTES, json);
            valid = jsonState != RecordState::Error &&
                    (jsonState != RecordState::Ready || parseJson(json, config));
            if (!valid)
                Log::warn(Log::STORAGE_TAG, "Legacy device JSON unreadable or invalid (state=%u)",
                          unsigned(jsonState));
            const auto wifiState = source.read(WIFI_CFG_FILE, 512, wifi);
            valid &= wifiState != RecordState::Error;
            if (wifiState == RecordState::Error)
                Log::warn(Log::STORAGE_TAG, "Legacy Wi-Fi file unreadable");
            if (wifiState == RecordState::Ready) {
                std::string ssid, password, line;
                std::istringstream stream(wifi);
                while (std::getline(stream, line)) {
                    if (!line.empty() && line.back() == '\r')
                        line.pop_back();
                    if (line.compare(0, 5, "ssid=") == 0)
                        ssid = line.substr(5);
                    else if (line.compare(0, 9, "password=") == 0)
                        password = line.substr(9);
                }
                const bool networkValid =
                    validNetwork(ssid, password) && (!ssid.empty() || password.empty());
                valid &= networkValid;
                if (networkValid && config.wifi.ssid.empty())
                    config.wifi = {ssid, password};
            }
        }
        const bool oldNvs = loadOldPreferences(config, oldUiNamespace, oldNetworkNamespace);
        if (!source.available) {
            if (oldNvs)
                remember(config);
            return oldNvs || (hasConfig && !std::strcmp(storageNamespace, NVS_NAMESPACE));
        }
        if (!valid || !writeRecord(config)) {
            remember(config);
            Log::warn(Log::STORAGE_TAG,
                      "Device settings migration incomplete; SD originals retained");
            return false;
        }
        Log::info(Log::STORAGE_TAG, "Device settings initialized in internal NVS");
    }
    remember(config);
    if (source.available)
        for (const char* path : {DEVICE_CONFIG_FILE, WIFI_CFG_FILE})
            if (!source.remove(path))
                Log::warn(Log::STORAGE_TAG, "Legacy settings cleanup pending: %s", path);
    return true;
}

std::string DeviceConfigRepository::toJson(const DeviceConfig& config, bool includeSecrets) const {
    JsonDocument doc;
    doc["version"] = 2;
    doc["deviceName"] = config.deviceName;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = config.wifi.ssid;
    if (includeSecrets)
        wifi["password"] = config.wifi.password;
    else {
        // Older portal builds merge API fields over a default empty password.
        // A null placeholder survives that merge and PUT keeps the saved secret.
        wifi["password"] = nullptr;
        wifi["passwordConfigured"] = !config.wifi.password.empty();
    }

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

bool DeviceConfigRepository::parseJson(const std::string& json, DeviceConfig& config) const {
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, json);
    if (error || !doc.is<JsonObject>())
        return false;

    config = defaultConfig();

    config.deviceName = doc["deviceName"].as<std::string>();

    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull()) {
        config.wifi.ssid = wifi["ssid"].as<std::string>();
        config.wifi.password = wifi["password"].as<std::string>();
    }

    JsonObject locale = doc["locale"];
    if (!locale.isNull()) {
        config.locale.language = locale["language"] | config.locale.language;
        config.locale.timezone = locale["timezone"] | config.locale.timezone;
        config.locale.dateFormat = locale["dateFormat"] | config.locale.dateFormat;
    }

    JsonObject ui = doc["ui"];
    if (!ui.isNull()) {
        config.ui.theme = ui["theme"] | config.ui.theme;
        config.ui.brightness = ui["brightness"] | 100;
    }

    JsonObject audio = doc["audio"];
    if (!audio.isNull()) {
        config.audio.volume = audio["volume"] | 70;
        config.audio.enabled = audio["enabled"] | true;
        if (!audio["volume"].isNull() || !audio["enabled"].isNull())
            legacyAudio(config.audio);
        if (auto music = audio["music"].as<JsonObject>()) {
            config.audio.independent = true;
            config.audio.music.enabled = music["enabled"] | config.audio.music.enabled;
            config.audio.music.volume =
                std::max(0, std::min(100, music["volume"] | int(config.audio.music.volume)));
        }
        if (auto effects = audio["effects"].as<JsonObject>()) {
            config.audio.independent = true;
            config.audio.effects.enabled = effects["enabled"] | config.audio.effects.enabled;
            config.audio.effects.volume =
                std::max(0, std::min(100, effects["volume"] | int(config.audio.effects.volume)));
        }
    }

    if (config.deviceName.empty())
        config.deviceName = defaultConfig().deviceName;

    config.locale.language = languageCode(config.locale.language);
    if (!validTheme(config.ui.theme))
        config.ui.theme = config.ui.theme == "blue"  ? "sky"
                          : config.ui.theme == "red" ? "coral"
                                                     : "mint";
    return true;
}
