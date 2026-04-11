#include "NetworkSystem.h"
#include "log/Logger.h"
#include <WiFi.h>
#include <sstream>
#include <algorithm>
#include "config/Assets.h"

NetworkSystem::NetworkSystem()
{
    status.state = State::Idle;
    status.message = "Wi-Fi idle";
}

void NetworkSystem::init(void *params)
{
    Log::info(Log::NETWORK_TAG, "Initializing network system");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);

    StorageSystem *storage = static_cast<StorageSystem *>(params);
    loadCredentials(storage);

    if (ssid.empty())
    {
        setState(State::Idle, "No Wi-Fi credentials found");
    }
    else
    {
        setState(State::Disconnected, "Ready to connect");
    }

    initialized = true;
}

void NetworkSystem::update()
{
    if (!initialized)
        return;

    refreshStatus();
}

bool NetworkSystem::connect()
{
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

void NetworkSystem::loadCredentials(StorageSystem *storage)
{
    if (!storage || !storage->isReady())
    {
        Log::warn(Log::NETWORK_TAG, "Storage unavailable, skipping Wi-Fi credentials load");
        return;
    }

    std::string contents;
    if (!storage->readTextFile(WIFI_CFG_FILE, contents))
    {
        Log::warn(Log::NETWORK_TAG, "Wi-Fi credentials file not found");
        return;
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

        auto trim = [](std::string &s)
        {
            size_t right = s.find_last_not_of(" \t\r");
            if (right == std::string::npos)
                s.clear();
            else
                s.erase(right + 1);

            size_t left = s.find_first_not_of(" \t");
            if (left == std::string::npos)
                s.clear();
            else
                s.erase(0, left);
        };

        trim(key);
        trim(value);

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
