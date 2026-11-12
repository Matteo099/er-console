#include "ConfigController.h"

#include <ArduinoJson.h>

ConfigController::ConfigController(PortalService& service) : service(service) {
}

void ConfigController::registerRoutes(WebServer& server) {
    server.on("/api/config", HTTP_GET, [&server, this]() {
        sendConfig(server);
    });
    server.on("/api/config", HTTP_PUT, [&server, this]() {
        updateConfig(server);
    });
    server.on("/api/config", HTTP_POST, [&server, this]() {
        resetConfig(server);
    });
}

void ConfigController::sendConfigPayload(WebServer& server, const DeviceConfig& config) {
    const auto body = DeviceConfigRepository().toJson(config);
    server.send(200, "application/json", body.c_str());
}

void ConfigController::sendConfig(WebServer& server) {
    sendConfigPayload(server, service.getConfig());
}

void ConfigController::updateConfig(WebServer& server) {
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
        server.send(400, "application/json", "{\"error\":\"invalid_json\"}");
        return;
    }

    DeviceConfig config = service.getConfig();
    if (!doc["deviceName"].isNull())
        config.deviceName = doc["deviceName"].as<std::string>();

    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull()) {
        config.wifi.ssid = wifi["ssid"].as<std::string>();
        config.wifi.password = wifi["password"].as<std::string>();
    }

    JsonObject locale = doc["locale"];
    if (!locale.isNull()) {
        config.locale.language = locale["language"].as<std::string>();
        config.locale.timezone = locale["timezone"].as<std::string>();
        config.locale.dateFormat = locale["dateFormat"].as<std::string>();
    }

    JsonObject ui = doc["ui"];
    if (!ui.isNull()) {
        config.ui.theme = ui["theme"].as<std::string>();
        config.ui.brightness = ui["brightness"] | 100;
    }

    JsonObject audio = doc["audio"];
    if (!audio.isNull()) {
        if (!audio["volume"].isNull() || !audio["enabled"].isNull()) {
            const int volume = audio["volume"] | config.audio.volume;
            if (volume < 0 || volume > 100) {
                server.send(400, "application/json", "{\"error\":\"invalid_volume\"}");
                return;
            }
            config.audio.music =
                config.audio.effects = {audio["enabled"] | config.audio.enabled, uint8_t(volume)};
        }
        for (const char* name : {"music", "effects"}) {
            if (audio[name].isNull())
                continue;
            if (!audio[name].is<JsonObject>()) {
                server.send(400, "application/json", "{\"error\":\"invalid_audio\"}");
                return;
            }
            auto& channel =
                std::string(name) == "music" ? config.audio.music : config.audio.effects;
            if ((!audio[name]["volume"].isNull() && !audio[name]["volume"].is<int>()) ||
                (!audio[name]["enabled"].isNull() && !audio[name]["enabled"].is<bool>())) {
                server.send(400, "application/json", "{\"error\":\"invalid_audio\"}");
                return;
            }
            const int volume = audio[name]["volume"] | int(channel.volume);
            if (volume < 0 || volume > 100) {
                server.send(400, "application/json", "{\"error\":\"invalid_volume\"}");
                return;
            }
            channel = {audio[name]["enabled"] | channel.enabled, uint8_t(volume)};
        }
        config.audio.enabled = config.audio.effects.enabled;
        config.audio.volume = config.audio.effects.volume;
    }

    if (!DeviceConfigRepository::validNetwork(config.wifi.ssid, config.wifi.password) ||
        (config.locale.language != "it" && config.locale.language != "it-IT" &&
         config.locale.language != "en" && config.locale.language != "en-GB" &&
         config.locale.language != "en-US")) {
        server.send(400, "application/json", "{\"error\":\"invalid_config\"}");
        return;
    }
    config.locale.language = config.locale.language.compare(0, 2, "en") == 0 ? "en-GB" : "it-IT";
    if (!service.saveConfig(config)) {
        server.send(500, "application/json", "{\"error\":\"save_failed\"}");
        return;
    }

    if (service.network())
        service.network()->reloadFromStorage(service.storage());

    sendConfigPayload(server, config);
}

void ConfigController::resetConfig(WebServer& server) {
    DeviceConfigRepository repository;
    DeviceConfig config = repository.defaultConfig();

    if (!service.saveConfig(config)) {
        server.send(500, "application/json", "{\"error\":\"save_failed\"}");
        return;
    }

    if (service.network())
        service.network()->reloadFromStorage(service.storage());

    sendConfigPayload(server, config);
}
