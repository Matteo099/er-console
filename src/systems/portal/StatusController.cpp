#include "StatusController.h"

#include <ArduinoJson.h>

StatusController::StatusController(PortalService& service)
    : service(service)
{
}

void StatusController::registerRoutes(WebServer& server)
{
    server.on("/api/status", HTTP_GET, [&server, this]() { sendStatus(server); });
    server.on("/api/system/reboot", HTTP_POST, [&server]() {
        server.send(200, "application/json", "{\"ok\":true}");
        delay(150);
        ESP.restart();
    });
}

void StatusController::sendStatus(WebServer& server)
{
    const PortalStatusData status = service.getStatus();

    DynamicJsonDocument doc(1024);
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["connected"] = status.wifiConnected;
    wifi["state"] = status.wifiState;
    wifi["ssid"] = status.wifiSsid;
    wifi["ip"] = status.wifiIp;

    JsonObject portal = doc["portal"].to<JsonObject>();
    portal["active"] = status.portalActive;
    portal["url"] = status.portalUrl;
    portal["ssid"] = status.apSsid;
    portal["ip"] = status.apIp;

    JsonObject hardware = doc["hardware"].to<JsonObject>();
    hardware["freeHeap"] = status.freeHeap;
    hardware["psramFree"] = status.psramFree;
    hardware["cpuFrequencyMhz"] = status.cpuFrequencyMhz;
    hardware["uptimeMs"] = status.uptimeMs;

    JsonObject storage = doc["storage"].to<JsonObject>();
    storage["totalBytes"] = status.sdTotalBytes;
    storage["usedBytes"] = status.sdUsedBytes;
    storage["freeBytes"] = status.sdFreeBytes;

    JsonObject battery = doc["battery"].to<JsonObject>();
    battery["supported"] = status.batterySupported;
    battery["percent"] = status.batteryPercent;
    battery["voltage"] = status.batteryVoltage;
    battery["current"] = status.batteryCurrent;

    JsonObject config = doc["config"].to<JsonObject>();
    config["deviceName"] = status.config.deviceName;
    config["language"] = status.config.locale.language;
    config["timezone"] = status.config.locale.timezone;
    config["dateFormat"] = status.config.locale.dateFormat;

    String body;
    serializeJsonPretty(doc, body);
    server.send(200, "application/json", body.c_str());
}