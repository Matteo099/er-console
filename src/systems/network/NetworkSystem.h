#pragma once

#include "core/SystemService.h"
#include <string>

class StorageSystem;

class NetworkSystem : public SystemService
{
public:
    enum class State
    {
        Idle,
        Connecting,
        Connected,
        Disconnected,
        Error
    };

    struct Status
    {
        std::string ssid;
        std::string ip;
        State state = State::Idle;
        std::string message;
    };

    NetworkSystem();

    void init(void* params = nullptr) override;
    void update() override;

    bool connect();
    void disconnect();

    const Status& getStatus() const { return status; }
    bool hasCredentials() const { return !ssid.empty(); }

private:
    void loadCredentials(StorageSystem* storage);
    void refreshStatus();
    void setState(State state, const std::string& message);

private:
    bool initialized = false;
    std::string ssid;
    std::string password;
    Status status;
    unsigned long connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 10000;
};
