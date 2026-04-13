#include "PortalService.h"

#include <Arduino.h>
#include <WiFi.h>

PortalService::PortalService(StorageSystem* storage, NetworkSystem* network)
    : storageSystem(storage), networkSystem(network)
{
}

void PortalService::setDependencies(StorageSystem* storage, NetworkSystem* network)
{
    storageSystem = storage;
    networkSystem = network;
}

DeviceConfig PortalService::getConfig() const
{
    DeviceConfig config = cachedConfig;
    if (storageSystem && storageSystem->isReady())
    {
        configRepository.load(storageSystem, config);
        cachedConfig = config;
    }
    return config;
}

bool PortalService::saveConfig(const DeviceConfig& config)
{
    if (!storageSystem || !storageSystem->isReady())
        return false;

    cachedConfig = config;
    return configRepository.save(storageSystem, config);
}

PortalStatusData PortalService::getStatus() const
{
    PortalStatusData status;
    status.config = getConfig();
    status.portalUrl = portalUrl();
    status.freeHeap = ESP.getFreeHeap();
    status.psramFree = ESP.getFreePsram();
    status.cpuFrequencyMhz = ESP.getCpuFreqMHz();
    status.uptimeMs = millis();

    if (storageSystem)
    {
        status.sdTotalBytes = storageSystem->totalBytes();
        status.sdUsedBytes = storageSystem->usedBytes();
        status.sdFreeBytes = storageSystem->freeBytes();
    }

    if (networkSystem)
    {
        const auto& networkStatus = networkSystem->getStatus();
        status.wifiState = networkStatus.message;
        status.wifiSsid = networkStatus.ssid;
        status.wifiIp = networkStatus.ip;
        status.wifiConnected = networkStatus.state == NetworkSystem::State::Connected;
    }

    if (WiFi.getMode() & WIFI_MODE_AP)
    {
        status.portalActive = true;
        status.apIp = WiFi.softAPIP().toString().c_str();
        status.apSsid = status.config.portal.apSsid;
    }

    return status;
}

std::vector<StorageEntry> PortalService::listFiles(const std::string& path) const
{
    std::vector<StorageEntry> entries;
    if (storageSystem && storageSystem->isReady())
        storageSystem->listDirectory(path.c_str(), entries, true);
    return entries;
}

bool PortalService::deletePath(const std::string& path) const
{
    return storageSystem && storageSystem->isReady() && storageSystem->removePath(path.c_str());
}

File PortalService::openUploadTarget(const std::string& path, bool overwrite) const
{
    if (!storageSystem || !storageSystem->isReady())
        return File();

    return storageSystem->openNative(path.c_str(), overwrite);
}

bool PortalService::ensureDirectory(const std::string& path) const
{
    return storageSystem && storageSystem->isReady() && storageSystem->createDirectory(path.c_str());
}

std::string PortalService::portalUrl() const
{
    if (WiFi.getMode() & WIFI_MODE_AP)
        return std::string("http://") + WiFi.softAPIP().toString().c_str() + "/";

    return "http://192.168.4.1/";
}

std::string PortalService::wifiQrPayload() const
{
    const DeviceConfig config = getConfig();
    std::string ssid = config.portal.apSsid.empty() ? "ER-Console-Setup" : config.portal.apSsid;
    std::string payload = "WIFI:S:" + ssid + ";T:nopass;P:;H:false;;";
    return payload;
}