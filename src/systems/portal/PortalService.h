#pragma once

#include <SD.h>
#include <string>
#include <vector>

#include "services/DeviceConfigRepository.h"
#include "systems/network/NetworkSystem.h"

struct PortalStatusData
{
    DeviceConfig config;
    std::string portalUrl;
    std::string wifiState;
    std::string wifiSsid;
    std::string wifiIp;
    bool wifiConnected = false;
    bool portalActive = false;
    std::string apSsid;
    std::string apIp;
    uint32_t freeHeap = 0;
    uint32_t psramFree = 0;
    uint32_t cpuFrequencyMhz = 0;
    uint32_t uptimeMs = 0;
    uint64_t sdTotalBytes = 0;
    uint64_t sdUsedBytes = 0;
    uint64_t sdFreeBytes = 0;
    bool batterySupported = false;
    std::string batteryPercent = "N/A";
    std::string batteryVoltage = "N/A";
    std::string batteryCurrent = "N/A";
};

struct WifiScanEntry
{
    std::string ssid;
    int signal = 0;
    bool secure = false;
    std::string authMode;
};

class PortalService
{
public:
    PortalService(StorageSystem* storage = nullptr, NetworkSystem* network = nullptr);
    void setDependencies(StorageSystem* storage, NetworkSystem* network);

    DeviceConfig getConfig() const;
    bool saveConfig(const DeviceConfig& config);
    PortalStatusData getStatus() const;
    std::vector<StorageEntry> listFiles(const std::string& path) const;
    bool deletePath(const std::string& path) const;
    bool movePath(const std::string& sourcePath, const std::string& destinationPath) const;
    File openUploadTarget(const std::string& path, bool overwrite = true) const;
    bool ensureDirectory(const std::string& path) const;
    std::vector<WifiScanEntry> scanWifiNetworks() const;

    std::string portalUrl() const;
    std::string portalSsid() const;
    std::string wifiQrPayload() const;

    StorageSystem* storage() const { return storageSystem; }
    NetworkSystem* network() const { return networkSystem; }

private:
    StorageSystem* storageSystem = nullptr;
    NetworkSystem* networkSystem = nullptr;
    DeviceConfigRepository configRepository;
    mutable DeviceConfig cachedConfig;
};