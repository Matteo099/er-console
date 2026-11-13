#include "ConfigStorageDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <ArduinoJson.h>
#include <nvs.h>
#include <cstring>
#include <map>
#include "services/DeviceConfigRepository.h"
#include "systems/portal/PortalService.h"
#include "core/SystemEventBus.h"

void configPersistenceDiagnostics(bool baseline) {
    DeviceConfig config;
    DeviceConfigRepository repository;
    const bool loaded = repository.load(nullptr, config);
    const auto current = repository.toJson(config, true);
    nvs_handle_t handle;
    if (!loaded || nvs_open("cfg-verify", NVS_READWRITE, &handle) != ESP_OK) {
        Serial.println("@CONFIGPERSIST ERROR");
        return;
    }
    if (baseline) {
        const bool saved =
            nvs_set_blob(handle, "baseline", current.data(), current.size()) == ESP_OK &&
            nvs_commit(handle) == ESP_OK;
        Serial.printf("@CONFIGBASELINE %s\n", saved ? "OK" : "ERROR");
    } else {
        size_t size = 0;
        bool matched = nvs_get_blob(handle, "baseline", nullptr, &size) == ESP_OK && size &&
                       size <= DeviceConfigRepository::MAX_RECORD_BYTES;
        std::string previous;
        if (matched) {
            previous.resize(size);
            matched = nvs_get_blob(handle, "baseline", &previous[0], &size) == ESP_OK &&
                      previous == current;
        }
        auto* storage = static_cast<StorageSystem*>(
            SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
        const bool clean = storage && storage->isReady() && !storage->exists(DEVICE_CONFIG_FILE) &&
                           !storage->exists(WIFI_CFG_FILE);
        const bool cleaned =
            matched && nvs_erase_key(handle, "baseline") == ESP_OK && nvs_commit(handle) == ESP_OK;
        Serial.printf("@CONFIGVERIFY match=%u clean_sd=%u baseline_cleared=%u\n", matched, clean,
                      cleaned);
    }
    nvs_close(handle);
}

void runConfigStorageDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@CONFIGCHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    // Only these isolated namespaces are ever erased. No user configuration,
    // credentials, preferences, statistics or SD files are modified by this test.
    const auto clear = []() {
        for (const char* name : {"cfg-check", "cfg-ui-check", "cfg-net-check"}) {
            nvs_handle_t handle;
            if (nvs_open(name, NVS_READWRITE, &handle) == ESP_OK) {
                nvs_erase_all(handle);
                nvs_commit(handle);
                nvs_close(handle);
            }
        }
    };
    const auto raw = [](const std::string& json) {
        nvs_handle_t handle;
        bool okay = nvs_open("cfg-check", NVS_READWRITE, &handle) == ESP_OK;
        if (okay) {
            okay = nvs_set_blob(handle, DeviceConfigRepository::NVS_KEY, json.data(),
                                json.size()) == ESP_OK &&
                   nvs_commit(handle) == ESP_OK;
            nvs_close(handle);
        }
        return okay;
    };
    clear();
    DeviceConfigRepository repository("cfg-check", "cfg-ui-check", "cfg-net-check");
    DeviceConfig config;
    check(!repository.load(nullptr, config) && config.ui.theme == "mint" &&
              config.wifi.ssid.empty(),
          "fresh device without SD uses defaults without preventing later migration");
    check(repository.readRecord(config) == DeviceConfigRepository::RecordState::Missing,
          "read-only startup does not write defaults before SD migration");

    DeviceConfig original;
    original.deviceName = "NVS test console";
    original.wifi = {"Test network", "test-only-secret"};
    original.locale.language = "en-GB";
    original.locale.timezone = "UTC";
    original.locale.dateFormat = "YYYY-MM-DD";
    original.ui.theme = "coral";
    original.ui.brightness = 63;
    original.audio.music = {false, 24};
    original.audio.effects = {true, 58};
    check(repository.save(nullptr, original), "complete configuration saves without an SD card");
    DeviceConfigRepository reopened("cfg-check", "cfg-ui-check", "cfg-net-check");
    check(reopened.load(nullptr, config) &&
              repository.toJson(config, true) == repository.toJson(original, true),
          "a reopened repository preserves every device setting");
    JsonDocument exposed;
    deserializeJson(exposed, repository.toJson(config));
    check(exposed["wifi"]["password"].isNull() && exposed["wifi"]["passwordConfigured"].as<bool>(),
          "public configuration never serializes the saved password");
    DeviceConfig stale;
    stale.ui.theme = "sky";
    stale.locale.language = "it-IT";
    stale.audio.music = {true, 81};
    stale.audio.effects = {false, 12};
    check(repository.savePreferences(stale) && reopened.load(nullptr, config) &&
              config.ui.theme == "sky" && config.audio.music.volume == 81 &&
              config.audio.effects.volume == 12 && config.deviceName == original.deviceName &&
              config.ui.brightness == 63 && config.locale.timezone == "UTC" &&
              config.wifi.password == original.wifi.password,
          "UI/audio updates preserve credentials and unrelated device settings");
    check(repository.saveNetworkPreferences("Open test", "") && reopened.load(nullptr, config) &&
              config.wifi.ssid == "Open test" && config.wifi.password.empty() &&
              config.ui.theme == "sky" && config.audio.music.volume == 81,
          "network-only updates preserve preferences and allow open networks");
    check(repository.saveNetworkPreferences("", "") && reopened.load(nullptr, config) &&
              config.wifi.ssid.empty(),
          "explicitly cleared credentials remain cleared");
    const auto before = repository.toJson(config, true);
    auto invalid = config;
    invalid.ui.brightness = 101;
    check(!repository.save(nullptr, invalid) && reopened.load(nullptr, config) &&
              repository.toJson(config, true) == before,
          "rejected settings do not change the committed snapshot");
    check(!repository.saveNetworkPreferences("Test", "short"), "invalid credentials are rejected");

    clear();
    std::map<std::string, std::string> files;
    unsigned reads = 0, removals = 0;
    bool cleanupBlocked = false, removedBeforeCommit = false;
    DeviceConfigRepository::LegacySource source;
    source.available = true;
    source.read = [&](const char* path, size_t limit, std::string& text) {
        ++reads;
        auto found = files.find(path);
        if (found == files.end())
            return DeviceConfigRepository::RecordState::Missing;
        if (found->second.size() > limit)
            return DeviceConfigRepository::RecordState::Error;
        text = found->second;
        return DeviceConfigRepository::RecordState::Ready;
    };
    source.remove = [&](const char* path) {
        DeviceConfig verified;
        removedBeforeCommit |=
            repository.readRecord(verified) != DeviceConfigRepository::RecordState::Ready;
        if (cleanupBlocked)
            return false;
        removals += files.erase(path);
        return true;
    };
    files[DEVICE_CONFIG_FILE] = repository.toJson(original, true);
    files[WIFI_CFG_FILE] = "ssid=Old fallback\r\npassword=fallback-secret\r\n";
    check(repository.loadLegacy(source, config) && config.deviceName == original.deviceName &&
              config.wifi.ssid == original.wifi.ssid && config.ui.theme == "coral",
          "SD JSON migration preserves settings and takes precedence over wifi.cfg");
    check(files.empty() && removals == 2 && !removedBeforeCommit,
          "SD originals are deleted only after the NVS snapshot can be read back");
    check(reopened.load(nullptr, config) && config.wifi.password == original.wifi.password,
          "migrated credentials remain readable with no SD present");
    files[DEVICE_CONFIG_FILE] = "{invalid JSON";
    const auto readCount = reads;
    check(reopened.loadLegacy(source, config) && reads == readCount &&
              config.deviceName == original.deviceName,
          "initialized NVS never imports a replaced card or rereads stale settings");

    clear();
    files.clear();
    files[WIFI_CFG_FILE] = "ssid=Legacy open\r\npassword=\r\n";
    check(repository.loadLegacy(source, config) && config.wifi.ssid == "Legacy open" &&
              config.wifi.password.empty() && files.empty(),
          "wifi.cfg-only migration accepts open networks and CRLF");

    clear();
    files[DEVICE_CONFIG_FILE] = repository.toJson(original, true);
    files[WIFI_CFG_FILE] = "";
    check(repository.loadLegacy(source, config) && files.empty() &&
              config.wifi.ssid == original.wifi.ssid,
          "empty legacy Wi-Fi file does not block configuration migration");

    clear();
    files[DEVICE_CONFIG_FILE] = "{invalid JSON";
    const auto deleted = removals;
    check(!repository.loadLegacy(source, config) && files.size() == 1 && removals == deleted &&
              repository.readRecord(config) == DeviceConfigRepository::RecordState::Missing,
          "invalid SD JSON is retained and never marks migration complete");
    files[DEVICE_CONFIG_FILE] = "[]";
    check(!repository.loadLegacy(source, config) && files.size() == 1 && removals == deleted,
          "non-object JSON is retained instead of being replaced by defaults");
    auto excessive = original;
    excessive.deviceName.assign(65, 'x');
    files[DEVICE_CONFIG_FILE] = repository.toJson(excessive, true);
    check(!repository.loadLegacy(source, config) && files.size() == 1 && removals == deleted,
          "failed configuration validation leaves SD originals intact");
    files[DEVICE_CONFIG_FILE] = repository.toJson(original, true);
    cleanupBlocked = true;
    check(repository.loadLegacy(source, config) && files.size() == 1,
          "a cleanup failure keeps a durable usable NVS configuration");
    cleanupBlocked = false;
    const auto retryReads = reads;
    check(reopened.loadLegacy(source, config) && files.empty() && reads == retryReads,
          "restart retries cleanup without reimporting SD values");

    clear();

    struct OldPreferences {
        uint32_t magic = 0x45525032;
        char theme[12] = "sky";
        char language[8] = "en-GB";
        uint8_t musicEnabled = 0, musicVolume = 19, effectsEnabled = 1, effectsVolume = 36;
    } oldUi;

    struct OldNetwork {
        uint32_t magic = 0x45524e31;
        char ssid[33] = "Saved NVS network";
        char password[65] = "saved-nvs-secret";
    } oldNet;

    nvs_handle_t handle;
    bool seeded = nvs_open("cfg-ui-check", NVS_READWRITE, &handle) == ESP_OK;
    if (seeded) {
        seeded = nvs_set_blob(handle, "prefs-v2", &oldUi, sizeof(oldUi)) == ESP_OK &&
                 nvs_commit(handle) == ESP_OK;
        nvs_close(handle);
    }
    bool networkSeeded = nvs_open("cfg-net-check", NVS_READWRITE, &handle) == ESP_OK;
    if (networkSeeded) {
        networkSeeded = nvs_set_blob(handle, "credentials", &oldNet, sizeof(oldNet)) == ESP_OK &&
                        nvs_commit(handle) == ESP_OK;
        nvs_close(handle);
    }
    files[DEVICE_CONFIG_FILE] = repository.toJson(original, true);
    check(seeded && networkSeeded && repository.loadLegacy(source, config) &&
              config.ui.theme == "sky" && config.audio.music.volume == 19 &&
              config.wifi.password == oldNet.password && config.deviceName == original.deviceName,
          "existing UI and network NVS records override old SD preferences");
    clear();
    bool oldSeeded = nvs_open("cfg-ui-check", NVS_READWRITE, &handle) == ESP_OK;
    if (oldSeeded) {
        oldSeeded = nvs_set_str(handle, "theme", "coral") == ESP_OK &&
                    nvs_set_u8(handle, "sound", 0) == ESP_OK && nvs_commit(handle) == ESP_OK;
        nvs_close(handle);
    }
    files[DEVICE_CONFIG_FILE] = "{}";
    check(oldSeeded && repository.loadLegacy(source, config) && config.ui.theme == "coral" &&
              !config.audio.music.enabled && !config.audio.effects.enabled,
          "first-generation theme and sound preferences also migrate");

    JsonDocument unsupported;
    deserializeJson(unsupported, repository.toJson(original, true));
    unsupported["version"] = 99;
    std::string bytes;
    serializeJson(unsupported, bytes);
    files[DEVICE_CONFIG_FILE] = repository.toJson(original, true);
    const auto beforeErrorReads = reads;
    check(raw(bytes) && !reopened.loadLegacy(source, config) && files.size() == 1 &&
              reads == beforeErrorReads,
          "unknown NVS schema is not overwritten or replaced from SD");
    check(!reopened.saveNetworkPreferences("Replacement", "replacement-secret"),
          "partial saves cannot overwrite an unreadable snapshot");
    check(raw(std::string(DeviceConfigRepository::MAX_RECORD_BYTES + 1, 'x')) &&
              reopened.readRecord(config) == DeviceConfigRepository::RecordState::Error,
          "oversized NVS data is rejected before allocation");
    DeviceConfigRepository unavailable("cfg-name-too-long", "cfg-ui-check", "cfg-net-check");
    const auto beforeUnavailable = removals;
    check(!unavailable.save(nullptr, original) && !unavailable.loadLegacy(source, config) &&
              removals == beforeUnavailable && files.size() == 1,
          "NVS open failures never delete or import SD originals");
    clear();
    PortalService service(nullptr, nullptr, nullptr);
    const auto effective = service.getConfig();
    DeviceConfig current;
    const bool ready =
        DeviceConfigRepository().readRecord(current) == DeviceConfigRepository::RecordState::Ready;
    check(ready && service.saveConfig(effective),
          "portal can save an unchanged configuration without SD");
    Serial.printf("@CONFIGTEST passed=%u failed=%u\n", passed, failed);
}
#endif
