#include "NetworkSystem.h"
#include "log/Logger.h"
#include <WiFi.h>
#include <sstream>
#include <algorithm>
#include "config/Assets.h"

namespace
{
    static void trimString(std::string& value)
    {
        size_t right = value.find_last_not_of(" \t\r");
        if (right == std::string::npos)
        {
            value.clear();
            return;
        }

        value.erase(right + 1);
        size_t left = value.find_first_not_of(" \t");
        if (left == std::string::npos)
            value.clear();
        else
            value.erase(0, left);
    }
}

NetworkSystem::NetworkSystem()
{
    status.state = State::Idle;
    status.message = "Wi-Fi idle";
}

void NetworkSystem::init(void *params)
{
    Log::info(Log::NETWORK_TAG, "Initializing network system");

#ifdef ER_DISABLE_WIFI
    WiFi.mode(WIFI_OFF);
    status.state = State::Idle;
    status.message = "Wi-Fi disabled for SD diagnostic";
    initialized = true;
    Log::warn(Log::NETWORK_TAG, "Wi-Fi disabled by build flag (SD diagnostic)");
    return;
#else
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(true, true);

    StorageSystem *storage = static_cast<StorageSystem *>(params);

    reloadFromStorage(storage);

    initialized = true;
#endif
}

void NetworkSystem::update()
{
    if (!initialized)
        return;

    refreshStatus();
}

bool NetworkSystem::connect()
{
#ifdef ER_DISABLE_WIFI
    Log::warn(Log::NETWORK_TAG, "Wi-Fi connect ignored: disabled by build flag");
    setState(State::Error, "Wi-Fi disabled");
    return false;
#else
    if (ssid.empty())
    {
        setState(State::Error, "Missing Wi-Fi credentials");
        return false;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        status.ip = WiFi.localIP().toString().c_str();
        setState(State::Connected, "Already connected");
        return true;
    }

    Log::info(Log::NETWORK_TAG, "Connecting to Wi-Fi '%s'", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    connectStartMs = millis();
    setState(State::Connecting, "Connecting...");
    return true;
#endif
}

bool NetworkSystem::reloadFromStorage(StorageSystem* storage)
{
    DeviceConfig config = configRepository.defaultConfig();
    bool loaded = storage && storage->isReady() && configRepository.load(storage, config);

    if (loaded)
    {
        setCredentials(config.wifi.ssid, config.wifi.password);
        if (!ssid.empty())
            Log::info(Log::NETWORK_TAG, "Wi-Fi credential loaded from config.json: %s", ssid.c_str());
    }
    else if (loadLegacyCredentials(storage))
    {
        config.wifi.ssid = ssid;
        config.wifi.password = password;
        if (storage && storage->isReady())
            configRepository.save(storage, config);
    }

    if (ssid.empty())
        setState(State::Idle, "No Wi-Fi credentials found");
    else if (WiFi.status() == WL_CONNECTED)
        setState(State::Connected, "Connected");
    else
        setState(State::Disconnected, "Ready to connect");

    return !ssid.empty();
}

void NetworkSystem::setCredentials(const std::string& newSsid, const std::string& newPassword)
{
    ssid = newSsid;
    password = newPassword;
}

void NetworkSystem::disconnect()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect(true, true);
        Log::info(Log::NETWORK_TAG, "Wi-Fi disconnected");
    }

    setState(State::Disconnected, "Disconnected");
}

bool NetworkSystem::loadLegacyCredentials(StorageSystem *storage)
{
    if (!storage || !storage->isReady())
    {
        Log::warn(Log::NETWORK_TAG, "Storage unavailable, skipping Wi-Fi credentials load");
        return false;
    }

    std::string contents;
    if (!storage->readTextFile(WIFI_CFG_FILE, contents))
    {
        Log::warn(Log::NETWORK_TAG, "Wi-Fi credentials file not found");
        return false;
    }

    std::istringstream stream(contents);
    std::string line;
    while (std::getline(stream, line))
    {
        auto sep = line.find('=');
        if (sep == std::string::npos)
            continue;

        std::string key = line.substr(0, sep);
        std::string value = line.substr(sep + 1);

        trimString(key);
        trimString(value);

        if (key == "ssid")
            ssid = value;
        else if (key == "password")
            password = value;
    }

    if (!ssid.empty())
    {
        Log::info(Log::NETWORK_TAG, "Wi-Fi credential loaded: %s", ssid.c_str());
    }
    else
    {
        Log::warn(Log::NETWORK_TAG, "Wi-Fi credentials are empty in file '%s'", WIFI_CFG_FILE);
    }

    return !ssid.empty();
}

void NetworkSystem::refreshStatus()
{
    wl_status_t wifiStatus = WiFi.status();
    if (status.state == State::Connecting)
    {
        if (wifiStatus == WL_CONNECTED)
        {
            status.ip = WiFi.localIP().toString().c_str();
            setState(State::Connected, "Connected");
            return;
        }

        if (millis() - connectStartMs > CONNECT_TIMEOUT_MS)
        {
            setState(State::Error, "Connection timed out");
            return;
        }
    }

    if (wifiStatus == WL_CONNECTED)
    {
        status.ip = WiFi.localIP().toString().c_str();
        setState(State::Connected, "Connected");
    }
    else if (status.state == State::Connected)
    {
        setState(State::Disconnected, "Disconnected");
    }
}

void NetworkSystem::setState(State newState, const std::string &message)
{
    status.state = newState;
    status.message = message;
    if (newState != State::Connected)
        status.ip.clear();
}
