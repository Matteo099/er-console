#include "NetworkSystem.h"
#include "config/Console.h"
#include "config/Assets.h"
#include "log/Logger.h"
#include <WiFi.h>
#include <sstream>

NetworkSystem::NetworkSystem() { status.message = "Idle"; }
void NetworkSystem::init(void *params)
{
    Log::info(Log::NETWORK_TAG, "Initializing network system");
#if WIFI_ENABLED
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    reloadFromStorage(static_cast<StorageSystem *>(params));
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
    Command next;
    std::string selectedSsid, selectedPassword;
    {
        TaskMutex::Guard lock(mutex);
        next = command; command = Command::None;
        selectedSsid = ssid; selectedPassword = password;
    }
    if (next == Command::Disconnect) WiFi.disconnect(false, false); // Keep the portal AP alive.
    if (next == Command::Connect)
    {
        WiFi.disconnect(false, false);
        WiFi.begin(selectedSsid.c_str(), selectedPassword.empty() ? nullptr : selectedPassword.c_str());
    }
    refreshStatus();
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
    command = Command::Connect; connectStartMs = millis();
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
    else if (status.state == State::Connecting && millis() - connectStartMs > CONNECT_TIMEOUT_MS)
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
