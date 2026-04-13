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
    server.on("/api/config", HTTP_POST, [&server, this]() { updateConfig(server); });
}

void ConfigController::sendConfig(WebServer& server)
{
    const DeviceConfig config = service.getConfig();
    DynamicJsonDocument doc(1024);
    doc["deviceName"] = config.deviceName;

    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = config.wifi.ssid;
    wifi["password"] = config.wifi.password;
    wifi["enabled"] = config.wifi.enabled;
    wifi["autoConnect"] = config.wifi.autoConnect;

    JsonObject locale = doc["locale"].to<JsonObject>();
    locale["language"] = config.locale.language;
    locale["timezone"] = config.locale.timezone;
    locale["dateFormat"] = config.locale.dateFormat;
    locale["latitude"] = config.locale.latitude;
    locale["longitude"] = config.locale.longitude;

    JsonObject portal = doc["portal"].to<JsonObject>();
    portal["apSsid"] = config.portal.apSsid;
    portal["apPassword"] = config.portal.apPassword;
    portal["enabled"] = config.portal.enabled;

    JsonObject ui = doc["ui"].to<JsonObject>();
    ui["theme"] = config.ui.theme;
    ui["brightness"] = config.ui.brightness;

    JsonObject audio = doc["audio"].to<JsonObject>();
    audio["volume"] = config.audio.volume;
    audio["enabled"] = config.audio.enabled;

    String body;
    serializeJsonPretty(doc, body);
    server.send(200, "application/json", body.c_str());
}

void ConfigController::updateConfig(WebServer& server)
{
    DynamicJsonDocument doc(1024);
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
        config.wifi.enabled = wifi["enabled"] | true;
        config.wifi.autoConnect = wifi["autoConnect"] | true;
    }

    JsonObject locale = doc["locale"];
    if (!locale.isNull())
    {
        config.locale.language = locale["language"].as<std::string>();
        config.locale.timezone = locale["timezone"].as<std::string>();
        config.locale.dateFormat = locale["dateFormat"].as<std::string>();
        config.locale.latitude = locale["latitude"] | 0.0;
        config.locale.longitude = locale["longitude"] | 0.0;
    }

    JsonObject portal = doc["portal"];
    if (!portal.isNull())
    {
        config.portal.apSsid = portal["apSsid"].as<std::string>();
        config.portal.apPassword = portal["apPassword"].as<std::string>();
        config.portal.enabled = portal["enabled"] | true;
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

    server.send(200, "application/json", "{\"ok\":true}");
}