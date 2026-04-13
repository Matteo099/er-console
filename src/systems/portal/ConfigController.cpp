#include "ConfigController.h"

#include <ArduinoJson.h>

ConfigController::ConfigController(PortalService& service)
    : service(service)
{
}

void ConfigController::registerRoutes(WebServer& server)
{
    server.on("/api/config", HTTP_GET, [&server, this]() { sendConfig(server); });
    server.on("/api/config", HTTP_PUT, [&server, this]() { updateConfig(server); });
    server.on("/api/config", HTTP_POST, [&server, this]() { resetConfig(server); });
}

void ConfigController::sendConfigPayload(WebServer& server, const DeviceConfig& config)
{
    JsonDocument doc;
    doc["deviceName"] = config.deviceName;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = config.wifi.ssid;
    wifi["password"] = config.wifi.password;

    JsonObject locale = doc["locale"].to<JsonObject>();
    locale["language"] = config.locale.language;
    locale["timezone"] = config.locale.timezone;
    locale["dateFormat"] = config.locale.dateFormat;

    JsonObject ui = doc["ui"].to<JsonObject>();
    ui["theme"] = config.ui.theme;
    ui["compactMode"] = false;
    ui["brightness"] = config.ui.brightness;

    JsonObject audio = doc["audio"].to<JsonObject>();
    audio["volume"] = config.audio.volume;
    audio["enabled"] = config.audio.enabled;

    String body;
    serializeJsonPretty(doc, body);
    server.send(200, "application/json", body.c_str());
}

void ConfigController::sendConfig(WebServer& server)
{
    sendConfigPayload(server, service.getConfig());
}

void ConfigController::updateConfig(WebServer& server)
{
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error)
    {
        server.send(400, "application/json", "{\"error\":\"invalid_json\"}");
        return;
    }

    DeviceConfig config = service.getConfig();
    config.deviceName = doc["deviceName"].as<std::string>();

    JsonObject wifi = doc["wifi"];
    if (!wifi.isNull())
    {
        config.wifi.ssid = wifi["ssid"].as<std::string>();
        config.wifi.password = wifi["password"].as<std::string>();
    }

    JsonObject locale = doc["locale"];
    if (!locale.isNull())
    {
        config.locale.language = locale["language"].as<std::string>();
        config.locale.timezone = locale["timezone"].as<std::string>();
        config.locale.dateFormat = locale["dateFormat"].as<std::string>();
    }

    JsonObject ui = doc["ui"];
    if (!ui.isNull())
    {
        config.ui.theme = ui["theme"].as<std::string>();
        config.ui.brightness = ui["brightness"] | 100;
    }

    JsonObject audio = doc["audio"];
    if (!audio.isNull())
    {
        config.audio.volume = audio["volume"] | 70;
        config.audio.enabled = audio["enabled"] | true;
    }

    if (!service.saveConfig(config))
    {
        server.send(500, "application/json", "{\"error\":\"save_failed\"}");
        return;
    }

    if (service.network())
        service.network()->reloadFromStorage(service.storage());

    sendConfigPayload(server, config);
}

void ConfigController::resetConfig(WebServer& server)
{
    DeviceConfigRepository repository;
    DeviceConfig config = repository.defaultConfig();

    if (!service.saveConfig(config))
    {
        server.send(500, "application/json", "{\"error\":\"save_failed\"}");
        return;
    }

    if (service.network())
        service.network()->reloadFromStorage(service.storage());

    sendConfigPayload(server, config);
}
