#include "PortalService.h"

#include <Arduino.h>
#include <WiFi.h>
#include <cstdio>

namespace {
static std::string trimCopy(std::string value) {
    auto left = value.find_first_not_of(" \t\r\n");
    if (left == std::string::npos)
        return {};

    auto right = value.find_last_not_of(" \t\r\n");
    return value.substr(left, right - left + 1);
}

}  // namespace

PortalService::PortalService(StorageSystem* storage, NetworkSystem* network, InputSystem* input)
    : storageSystem(storage), networkSystem(network), inputSystem(input) {
}

void PortalService::setDependencies(StorageSystem* storage, NetworkSystem* network,
                                    InputSystem* input) {
    storageSystem = storage;
    networkSystem = network;
    inputSystem = input;
}

DeviceConfig PortalService::getConfig() const {
    DeviceConfig config = cachedConfig;
    if (storageSystem && storageSystem->isReady()) {
        configRepository.load(storageSystem, config);
        cachedConfig = config;
    } else
        configRepository.loadCached(config);
    return config;
}

bool PortalService::saveConfig(const DeviceConfig& config) {
    if (!storageSystem || !storageSystem->isReady())
        return false;

    if (!configRepository.save(storageSystem, config))
        return false;
    cachedConfig = config;
    return true;
}

PortalStatusData PortalService::getStatus() const {
    PortalStatusData status;
    status.config = getConfig();
    status.portalUrl = portalUrl();
    status.freeHeap = ESP.getFreeHeap();
    status.psramFree = ESP.getFreePsram();
    status.cpuFrequencyMhz = ESP.getCpuFreqMHz();
    status.uptimeMs = millis();

    if (storageSystem) {
        status.sdTotalBytes = storageSystem->totalBytes();
        status.sdUsedBytes = storageSystem->usedBytes();
        status.sdFreeBytes = storageSystem->freeBytes();
    }

    if (networkSystem) {
        const auto& networkStatus = networkSystem->getStatus();
        status.wifiState = networkStatus.message;
        status.wifiSsid = networkStatus.ssid;
        status.wifiIp = networkStatus.ip;
        status.wifiConnected = networkStatus.state == NetworkSystem::State::Connected;
    }

    if (WiFi.getMode() & WIFI_MODE_AP) {
        status.portalActive = true;
        status.apIp = WiFi.softAPIP().toString().c_str();
        status.apSsid = portalSsid();
    }

    if (inputSystem && inputSystem->power()) {
        const auto metrics = inputSystem->power()->getMetrics();
        status.batterySupported = inputSystem->power()->isSupported();
        status.batteryAvailable = metrics.available;
        status.batteryOverflow = metrics.overflow;
        status.batterySampleTimeMs = metrics.sampleTimeMs;
        status.batteryVoltageV = metrics.busVoltageV;
        status.batteryCurrentMa = metrics.currentMa;
        status.batteryPowerMw = metrics.powerMw;
        status.batteryLoadVoltageV = metrics.loadVoltageV;
        status.batteryShuntVoltageMv = metrics.shuntVoltageMv;

        if (metrics.available) {
            char buffer[32];

            std::snprintf(buffer, sizeof(buffer), "%.2f V", metrics.busVoltageV);
            status.batteryVoltage = buffer;

            std::snprintf(buffer, sizeof(buffer), "%.2f mA", metrics.currentMa);
            status.batteryCurrent = buffer;

            std::snprintf(buffer, sizeof(buffer), "%.2f mW", metrics.powerMw);
            status.batteryPower = buffer;

            std::snprintf(buffer, sizeof(buffer), "%.2f V", metrics.loadVoltageV);
            status.batteryLoadVoltage = buffer;

            std::snprintf(buffer, sizeof(buffer), "%.2f mV", metrics.shuntVoltageMv);
            status.batteryShuntVoltage = buffer;

            status.batteryPercent = "N/A";
        }
    }

    return status;
}

std::vector<StorageEntry> PortalService::listFiles(const std::string& path) const {
    std::vector<StorageEntry> entries;
    if (storageSystem && storageSystem->isReady())
        storageSystem->listDirectory(path.c_str(), entries, true);
    return entries;
}

bool PortalService::deletePath(const std::string& path) const {
    return storageSystem && storageSystem->isReady() && storageSystem->removePath(path.c_str());
}

bool PortalService::movePath(const std::string& sourcePath,
                             const std::string& destinationPath) const {
    return storageSystem && storageSystem->isReady() &&
           storageSystem->movePath(sourcePath.c_str(), destinationPath.c_str(), true);
}

File PortalService::openUploadTarget(const std::string& path, bool overwrite) const {
    if (!storageSystem || !storageSystem->isReady())
        return File();

    return storageSystem->openNative(path.c_str(), overwrite);
}

bool PortalService::ensureDirectory(const std::string& path) const {
    return storageSystem && storageSystem->isReady() &&
           storageSystem->createDirectory(path.c_str());
}

std::vector<WifiScanEntry> PortalService::scanWifiNetworks() const {
    if (!networkSystem)
        return {};
    networkSystem->requestScan();
    const auto started = millis();
    auto scan = networkSystem->getScanSnapshot();
    // Preserve the synchronous HTTP contract while the network task owns the
    // asynchronous SDK scan. LVGL keeps running on its own task.
    while (scan.state == WifiScanState::Scanning && millis() - started < 10000) {
        vTaskDelay(pdMS_TO_TICKS(20));
        networkSystem->scanSnapshot(scan.revision, scan);
    }
    return scan.networks;
}

std::string PortalService::portalUrl() const {
    if (WiFi.getMode() & WIFI_MODE_AP)
        return std::string("http://") + WiFi.softAPIP().toString().c_str() + "/";

    return "http://192.168.4.1/";
}

std::string PortalService::portalSsid() const {
    DeviceConfig config = getConfig();
    std::string deviceName = trimCopy(config.deviceName);
    if (deviceName.empty())
        deviceName = DeviceConfigRepository().defaultConfig().deviceName;

    return deviceName + " ER-WiFi";
}

std::string PortalService::wifiQrPayload() const {
    // Encode the AP actually running, not a fresh (possibly failed) SD config read.
    const String ssid = WiFi.softAPSSID();
    if (ssid.isEmpty())
        return {};
    std::string escaped;
    for (const char value : std::string(ssid.c_str())) {
        if (value == '\\' || value == ';' || value == ',' || value == ':' || value == '"')
            escaped += '\\';
        escaped += value;
    }
    return "WIFI:T:nopass;S:" + escaped + ";H:false;;";
}
