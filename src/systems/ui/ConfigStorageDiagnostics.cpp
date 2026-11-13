#include "ConfigStorageDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <ArduinoJson.h>
#include <nvs.h>
#include "services/DeviceConfigRepository.h"

void runConfigStorageDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@CONFIGCHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    const auto clear = []() {
        nvs_handle_t handle;
        if (nvs_open("cfg-check", NVS_READWRITE, &handle) == ESP_OK) {
            nvs_erase_all(handle);
            nvs_commit(handle);
            nvs_close(handle);
        }
    };
    // Only this test namespace is modified, never the user's configuration.
    clear();
    DeviceConfigRepository repository("cfg-check");
    DeviceConfig config;
    check(!repository.load(config) && config.ui.theme == "mint" &&
              config.locale.language == "it-IT" && config.wifi.ssid.empty(),
          "missing NVS record uses device defaults");
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
    check(repository.save(original), "complete configuration saves to NVS");
    DeviceConfigRepository reopened("cfg-check");
    check(reopened.load(config) &&
              repository.toJson(config, true) == repository.toJson(original, true),
          "reopened repository preserves all settings");
    JsonDocument exposed;
    deserializeJson(exposed, repository.toJson(config));
    check(exposed["wifi"]["password"].isUnbound() &&
              exposed["wifi"]["passwordConfigured"].as<bool>(),
          "public serialization omits the saved password");
    DeviceConfig preferences;
    preferences.ui.theme = "sky";
    preferences.audio.music = {true, 81};
    preferences.audio.effects = {false, 12};
    const bool preferencesSaved = repository.savePreferences(preferences);
    check(preferencesSaved && reopened.load(config) && config.ui.theme == "sky" &&
              config.audio.music.volume == 81 && config.audio.music.enabled &&
              !config.audio.effects.enabled && config.deviceName == original.deviceName &&
              config.ui.brightness == 63 && config.locale.timezone == "UTC" &&
              config.wifi.password == original.wifi.password,
          "preference updates preserve network and device settings");
    check(repository.saveNetworkPreferences("Open test", "") && reopened.load(config) &&
              config.wifi.ssid == "Open test" && config.wifi.password.empty() &&
              config.ui.theme == "sky" && config.audio.music.volume == 81,
          "network updates preserve preferences and support open networks");
    check(repository.saveNetworkPreferences("", "") && reopened.load(config) &&
              config.wifi.ssid.empty(),
          "cleared credentials remain cleared");
    const auto before = repository.toJson(config, true);
    auto invalid = config;
    invalid.ui.brightness = 101;
    check(!repository.save(invalid) && reopened.load(config) &&
              repository.toJson(config, true) == before,
          "invalid settings leave the saved record unchanged");
    check(!repository.saveNetworkPreferences("Test", "short"), "invalid credentials are rejected");
    invalid = config;
    invalid.locale.language = "en-US";
    check(!repository.save(invalid), "only configured language codes are accepted");

    nvs_handle_t handle;
    bool corrupted = nvs_open("cfg-check", NVS_READWRITE, &handle) == ESP_OK;
    if (corrupted) {
        const char bytes[] = "invalid";
        corrupted = nvs_set_blob(handle, DeviceConfigRepository::NVS_KEY, bytes,
                                 sizeof(bytes) - 1) == ESP_OK &&
                    nvs_commit(handle) == ESP_OK;
        nvs_close(handle);
    }
    check(corrupted && !reopened.load(config) &&
              !reopened.saveNetworkPreferences("Replacement", "replacement-secret"),
          "unreadable data is rejected instead of overwritten by partial saves");
    clear();
    Serial.printf("@CONFIGTEST passed=%u failed=%u\n", passed, failed);
}
#endif
