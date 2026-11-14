#include "DeviceConfigRepository.h"
#include <ArduinoJson.h>
#include <nvs.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstring>
#include <utility>
#include "core/TaskMutex.h"
#include "log/Logger.h"

namespace {
TaskMutex configMutex;
std::atomic<uint32_t> preferencesRevision{0};

bool validTheme(const std::string& value) {
    return value == "mint" || value == "sky" || value == "coral";
}
}  // namespace

DeviceConfig DeviceConfigRepository::defaultConfig() const {
    return DeviceConfig{};
}

uint32_t DeviceConfigRepository::revision() {
    return preferencesRevision.load();
}

bool DeviceConfigRepository::load(DeviceConfig& config) const {
    TaskMutex::Guard lock(configMutex);
    config = defaultConfig();
    const auto state = readRecord(config);
    if (state == RecordState::Error)
        Log::error(Log::STORAGE_TAG, "Device NVS configuration unreadable");
    return state == RecordState::Ready;
}

bool DeviceConfigRepository::save(const DeviceConfig& config) const {
    TaskMutex::Guard lock(configMutex);
    return writeRecord(config);
}

bool DeviceConfigRepository::savePreferences(const DeviceConfig& config) const {
    TaskMutex::Guard lock(configMutex);
    DeviceConfig current;
    if (readRecord(current) == RecordState::Error)
        return false;
    current.ui.theme = config.ui.theme;
    current.locale.language = config.locale.language;
    current.audio = config.audio;
    return writeRecord(current);
}

bool DeviceConfigRepository::saveNetworkPreferences(const std::string& ssid,
                                                    const std::string& password) const {
    TaskMutex::Guard lock(configMutex);
    DeviceConfig config;
    if (readRecord(config) == RecordState::Error)
        return false;
    config.wifi = {ssid, password};
    return writeRecord(config);
}

bool DeviceConfigRepository::validNetwork(const std::string& ssid, const std::string& password) {
    if (ssid.size() > 32 || password.size() > 64 || ssid.find('\0') != std::string::npos ||
        password.find('\0') != std::string::npos || (ssid.empty() && !password.empty()))
        return false;
    if (password.size() == 64)
        return std::all_of(password.begin(), password.end(), [](unsigned char c) {
            return std::isxdigit(c);
        });
    return password.empty() || (password.size() >= 8 && password.size() <= 63);
}

bool DeviceConfigRepository::validConfig(const DeviceConfig& config) {
    const auto text = [](const std::string& value, size_t maximum) {
        return !value.empty() && value.size() <= maximum && value.find('\0') == std::string::npos;
    };
    return text(config.deviceName, 64) && text(config.locale.timezone, 64) &&
           text(config.locale.dateFormat, 32) && validTheme(config.ui.theme) &&
           config.ui.brightness >= 0 && config.ui.brightness <= 100 &&
           config.audio.music.volume <= 100 && config.audio.effects.volume <= 100 &&
           (config.locale.language == "it-IT" || config.locale.language == "en-GB") &&
           validNetwork(config.wifi.ssid, config.wifi.password);
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
    return result == ESP_OK && parseJson(json, config) ? RecordState::Ready : RecordState::Error;
}

bool DeviceConfigRepository::writeRecord(const DeviceConfig& config) const {
    if (!validConfig(config))
        return false;
    const auto json = toJson(config, true);
    if (json.size() > MAX_RECORD_BYTES)
        return false;
    DeviceConfig previous;
    if (readRecord(previous) == RecordState::Ready && toJson(previous, true) == json)
        return true;
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
    const auto readState = readRecord(verified);
    const auto verifiedJson = toJson(verified, true);
    if (result != ESP_OK || readState != RecordState::Ready || verifiedJson != json) {
        Log::error(Log::STORAGE_TAG, "NVS settings save/verification failed: %s",
                   esp_err_to_name(result));
        return false;
    }
    if (!std::strcmp(storageNamespace, NVS_NAMESPACE))
        preferencesRevision.fetch_add(1);
    return true;
}

std::string DeviceConfigRepository::toJson(const DeviceConfig& config, bool includeSecrets) const {
    JsonDocument doc;
    doc["version"] = 2;
    doc["deviceName"] = config.deviceName;
    doc["wifi"]["ssid"] = config.wifi.ssid;
    if (includeSecrets)
        doc["wifi"]["password"] = config.wifi.password;
    else
        doc["wifi"]["passwordConfigured"] = !config.wifi.password.empty();
    doc["locale"]["language"] = config.locale.language;
    doc["locale"]["timezone"] = config.locale.timezone;
    doc["locale"]["dateFormat"] = config.locale.dateFormat;
    doc["ui"]["theme"] = config.ui.theme;
    doc["ui"]["brightness"] = config.ui.brightness;
    doc["audio"]["music"]["enabled"] = config.audio.music.enabled;
    doc["audio"]["music"]["volume"] = config.audio.music.volume;
    doc["audio"]["effects"]["enabled"] = config.audio.effects.enabled;
    doc["audio"]["effects"]["volume"] = config.audio.effects.volume;
    std::string output;
    serializeJson(doc, output);
    return output;
}

bool DeviceConfigRepository::parseJson(const std::string& json, DeviceConfig& config) const {
    JsonDocument doc;
    if (deserializeJson(doc, json) || !doc["version"].is<unsigned>() ||
        doc["version"].as<unsigned>() != 2 || !doc["deviceName"].is<const char*>() ||
        !doc["wifi"]["ssid"].is<const char*>() || !doc["wifi"]["password"].is<const char*>() ||
        !doc["locale"]["language"].is<const char*>() ||
        !doc["locale"]["timezone"].is<const char*>() ||
        !doc["locale"]["dateFormat"].is<const char*>() || !doc["ui"]["theme"].is<const char*>() ||
        !doc["ui"]["brightness"].is<int>())
        return false;
    DeviceConfig decoded;
    decoded.deviceName = doc["deviceName"].as<std::string>();
    decoded.wifi = {doc["wifi"]["ssid"].as<std::string>(),
                    doc["wifi"]["password"].as<std::string>()};
    decoded.locale.language = doc["locale"]["language"].as<std::string>();
    decoded.locale.timezone = doc["locale"]["timezone"].as<std::string>();
    decoded.locale.dateFormat = doc["locale"]["dateFormat"].as<std::string>();
    decoded.ui.theme = doc["ui"]["theme"].as<std::string>();
    decoded.ui.brightness = doc["ui"]["brightness"].as<int>();
    for (const char* name : {"music", "effects"}) {
        auto channel = doc["audio"][name].as<JsonObjectConst>();
        if (!channel["enabled"].is<bool>() || !channel["volume"].is<unsigned>() ||
            channel["volume"].as<unsigned>() > 100)
            return false;
        auto& target = name[0] == 'm' ? decoded.audio.music : decoded.audio.effects;
        target.enabled = channel["enabled"].as<bool>();
        target.volume = channel["volume"].as<uint8_t>();
    }
    if (!validConfig(decoded))
        return false;
    config = std::move(decoded);
    return true;
}
