#include "StatusController.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include "log/Logger.h"

namespace
{
    static const char* wifiStateToApiValue(NetworkSystem::State state)
    {
        switch (state)
        {
            case NetworkSystem::State::Connected: return "connected";
            case NetworkSystem::State::Connecting: return "connecting";
            case NetworkSystem::State::Disconnected: return "disconnected";
            case NetworkSystem::State::Error: return "error";
            case NetworkSystem::State::Idle:
            default:
                return "idle";
        }
    }
}

StatusController::StatusController(PortalService& service)
    : service(service)
{
}

void StatusController::registerRoutes(WebServer& server)
{
    server.on("/api/status", HTTP_GET, [&server, this]() { sendStatus(server); });
    server.on("/api/wifi/scan", HTTP_GET, [&server, this]() { scanWifi(server); });
    server.on("/api/system/reboot", HTTP_POST, [&server, this]() { rebootSystem(server); });
    server.on("/api/system/shutdown", HTTP_POST, [&server, this]() { shutdownSystem(server); });
    server.on("/api/system/logs", HTTP_GET, [&server, this]() { getLogs(server); });
    server.on("/api/system/logs", HTTP_DELETE, [&server, this]() { clearLogs(server); });
}

void StatusController::sendStatus(WebServer& server)
{
    const PortalStatusData status = service.getStatus();

    JsonDocument doc;
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["connected"] = status.wifiConnected;
    wifi["state"] = service.network() ? wifiStateToApiValue(service.network()->getStatus().state) : "idle";
    wifi["message"] = status.wifiState;
    wifi["ssid"] = status.wifiSsid;
    wifi["ip"] = status.wifiIp;
    wifi["signal"] = status.wifiConnected ? WiFi.RSSI() : 0;

    JsonObject portal = doc["portal"].to<JsonObject>();
    portal["active"] = status.portalActive;
    portal["url"] = status.portalUrl;
    portal["ssid"] = status.apSsid;
    portal["ip"] = status.apIp;
    portal["qrPayload"] = service.wifiQrPayload();

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
    battery["available"] = status.batteryAvailable;
    battery["overflow"] = status.batteryOverflow;
    battery["sampleTimeMs"] = status.batterySampleTimeMs;
    battery["voltageV"] = status.batteryVoltageV;
    battery["currentMa"] = status.batteryCurrentMa;
    battery["powerMw"] = status.batteryPowerMw;
    battery["loadVoltageV"] = status.batteryLoadVoltageV;
    battery["shuntVoltageMv"] = status.batteryShuntVoltageMv;
    battery["percent"] = status.batteryPercent;
    battery["voltage"] = status.batteryVoltage;
    battery["current"] = status.batteryCurrent;
    battery["power"] = status.batteryPower;
    battery["loadVoltage"] = status.batteryLoadVoltage;
    battery["shuntVoltage"] = status.batteryShuntVoltage;

    JsonObject config = doc["config"].to<JsonObject>();
    config["deviceName"] = status.config.deviceName;
    config["language"] = status.config.locale.language;
    config["timezone"] = status.config.locale.timezone;
    config["dateFormat"] = status.config.locale.dateFormat;

    String body;
    serializeJsonPretty(doc, body);
    server.send(200, "application/json", body.c_str());
}

void StatusController::scanWifi(WebServer& server)
{
    const std::vector<WifiScanEntry> networks = service.scanWifiNetworks();

    JsonDocument doc;
    JsonArray items = doc["networks"].to<JsonArray>();
    for (const WifiScanEntry& network : networks)
    {
        JsonObject item = items.add<JsonObject>();
        item["ssid"] = network.ssid;
        item["signal"] = network.signal;
        item["secure"] = network.secure;
        item["authMode"] = network.authMode;
    }

    String body;
    serializeJsonPretty(doc, body);
    server.send(200, "application/json", body.c_str());
}

void StatusController::getLogs(WebServer& server)
{
    std::size_t lines = 100;
    if (server.hasArg("lines"))
    {
        long requested = server.arg("lines").toInt();
        if (requested > 0)
            lines = static_cast<std::size_t>(requested);
    }

    const std::string logs = Log::getRecentLogs(lines);
    server.send(200, "text/plain", logs.c_str());
}

void StatusController::clearLogs(WebServer& server)
{
    Log::clearRecentLogs();
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Logs cleared\"}");
}

void StatusController::rebootSystem(WebServer& server)
{
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Reboot scheduled\"}");
    delay(150);
    ESP.restart();
}

void StatusController::shutdownSystem(WebServer& server)
{
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Shutdown scheduled\"}");
    delay(150);
    esp_deep_sleep_start();
}
