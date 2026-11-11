#include "NetworkSystem.h"
#include "config/Console.h"
#include "config/Assets.h"
#include "log/Logger.h"
#include <WiFi.h>
#include <sstream>

namespace
{
    const char *authModeName(wifi_auth_mode_t mode)
    {
        switch (mode)
        {
        case WIFI_AUTH_OPEN: return "open";
        case WIFI_AUTH_WEP: return "wep";
        case WIFI_AUTH_WPA_PSK: return "wpa";
        case WIFI_AUTH_WPA2_PSK: return "wpa2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "wpa_wpa2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "wpa2_enterprise";
        case WIFI_AUTH_WPA3_PSK: return "wpa3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "wpa2_wpa3";
        default: return "unknown";
        }
    }
}

NetworkSystem::NetworkSystem() { status.message = "Idle"; }
void NetworkSystem::init(void *params)
{
    Log::info(Log::NETWORK_TAG, "Initializing network system");
#if WIFI_ENABLED
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    if (reloadFromStorage(static_cast<StorageSystem *>(params)))
    {
        // Queue association for update(); boot must not wait for the network.
        connect();
    }
#else
    WiFi.mode(WIFI_OFF);
    setState(State::Idle, "Disabled by build configuration");
    Log::warn(Log::NETWORK_TAG, "Wi-Fi disabled in Console.h");
#endif
    initialized = true;
}
void NetworkSystem::update()
{
#if WIFI_ENABLED
    if (!initialized) return;
    updateScan();
    Command next;
    std::string selectedSsid, selectedPassword;
    {
        TaskMutex::Guard lock(mutex);
        // The network task alone owns Arduino's scan result buffer. Finish an
        // active scan before association so portal/UI requests cannot race it.
        if (scanActive) return;
        next = command; command = Command::None;
        selectedSsid = ssid; selectedPassword = password;
    }
    if (next == Command::Disconnect) WiFi.disconnect(false, false); // Keep the portal AP alive.
    if (next == Command::Connect)
    {
        WiFi.disconnect(false, false);
        WiFi.begin(selectedSsid.c_str(), selectedPassword.empty() ? nullptr : selectedPassword.c_str());
        TaskMutex::Guard lock(mutex);
        connectStartMs = millis();
    }
    refreshStatus();
#endif
}
bool NetworkSystem::requestScan()
{
    TaskMutex::Guard lock(mutex);
#if !WIFI_ENABLED
    scan.state = WifiScanState::Disabled; ++scan.revision; return false;
#else
    if (scanRequested || scanActive) return true;
    if (status.state == State::Connecting || command == Command::Connect)
    {
        scan.state = WifiScanState::Busy; ++scan.revision; return false;
    }
    scanRequested = true;
    scan.state = WifiScanState::Scanning; ++scan.revision;
    return true;
#endif
}
void NetworkSystem::updateScan()
{
#if WIFI_ENABLED
    TaskMutex::Guard lock(mutex);
    if (scanRequested && !scanActive)
    {
        scanRequested = false;
        if (command != Command::None)
        {
            scan.state = WifiScanState::Busy; ++scan.revision; return;
        }
        scanActive = true;
        const int result = WiFi.scanNetworks(true, false);
        if (result == WIFI_SCAN_RUNNING) return;
        if (result < 0)
        {
            scanActive = false; scan.state = WifiScanState::Error; ++scan.revision; return;
        }
    }
    if (!scanActive) return;
    const int count = WiFi.scanComplete();
    if (count == WIFI_SCAN_RUNNING) return;
    if (count < 0) scan.state = WifiScanState::Error;
    else
    {
        std::vector<WifiScanEntry> results;
        for (int i=0;i<count;++i)
        {
            WifiScanEntry entry;
            entry.ssid = WiFi.SSID(i).c_str(); entry.signal = WiFi.RSSI(i);
            entry.secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
            entry.authMode = authModeName(WiFi.encryptionType(i));
            results.push_back(entry);
        }
        WifiScan::normalize(results);
        scan.networks.swap(results);
        scan.state = WifiScanState::Ready;
    }
    WiFi.scanDelete();
    scanActive = false; ++scan.revision;
#endif
}
bool NetworkSystem::connect()
{
    TaskMutex::Guard lock(mutex);
#if !WIFI_ENABLED
    setState(State::Error, "Disabled by build configuration"); return false;
#else
    if (ssid.empty()) { setState(State::Error, "Missing credentials"); return false; }
    status.ssid = ssid; status.ip.clear(); status.signal = 0;
    command = Command::Connect; connectStartMs = 0; // Association timeout excludes time spent waiting for scan.
    setState(State::Connecting, "Connecting"); return true;
#endif
}
void NetworkSystem::disconnect()
{
    TaskMutex::Guard lock(mutex);
    command = Command::Disconnect;
    setState(State::Disconnected, "Disconnected");
}
void NetworkSystem::setCredentials(const std::string &newSsid, const std::string &newPassword)
{
    TaskMutex::Guard lock(mutex);
    ssid = newSsid; password = newPassword;
    if (status.state != State::Connected) status.ssid = ssid;
}
bool NetworkSystem::reloadFromStorage(StorageSystem *storage)
{
    DeviceConfig config;
    const bool loaded = configRepository.load(storage, config);
    setCredentials(config.wifi.ssid, config.wifi.password);
    if (!loaded && config.wifi.ssid.empty()) loadLegacyCredentials(storage);
    setState(hasCredentials() ? State::Disconnected : State::Idle, hasCredentials() ? "Ready to connect" : "Missing credentials");
    return hasCredentials();
}
bool NetworkSystem::loadLegacyCredentials(StorageSystem *storage)
{
    std::string contents;
    if (!storage || !storage->readTextFile(WIFI_CFG_FILE, contents)) return false;
    std::string selectedSsid, selectedPassword, line;
    std::istringstream stream(contents);
    while (std::getline(stream, line))
    {
        const auto separator = line.find('='); if (separator == std::string::npos) continue;
        auto value = line.substr(separator + 1);
        if (!value.empty() && value.back() == '\r') value.pop_back();
        if (line.substr(0, separator) == "ssid") selectedSsid = value;
        if (line.substr(0, separator) == "password") selectedPassword = value;
    }
    if (selectedSsid.empty() || !DeviceConfigRepository::validNetwork(selectedSsid, selectedPassword)) return false;
    setCredentials(selectedSsid, selectedPassword);
    configRepository.saveNetworkPreferences(selectedSsid, selectedPassword);
    return true;
}
void NetworkSystem::refreshStatus()
{
    TaskMutex::Guard lock(mutex);
    if (WiFi.status() == WL_CONNECTED)
    {
        status.ssid = WiFi.SSID().c_str(); status.ip = WiFi.localIP().toString().c_str(); status.signal = WiFi.RSSI();
        setState(State::Connected, "Connected");
    }
    else if (status.state == State::Connecting && connectStartMs && millis() - connectStartMs > CONNECT_TIMEOUT_MS)
        setState(State::Error, "Connection timed out", true);
    else if (status.state == State::Connected) setState(State::Disconnected, "Disconnected");
}
void NetworkSystem::setState(State state, const std::string &message, bool timedOut)
{
    TaskMutex::Guard lock(mutex);
    status.state = state; status.message = message;
    status.timedOut = timedOut;
    if (state != State::Connected) { status.ip.clear(); status.signal = 0; }
}
