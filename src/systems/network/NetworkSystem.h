#pragma once

#include "systems/storage/StorageSystem.h"
#include "core/SystemService.h"
#include "services/DeviceConfigRepository.h"
#include <string>

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
    bool reloadFromStorage(StorageSystem* storage);
    void setCredentials(const std::string& newSsid, const std::string& newPassword);

    const Status& getStatus() const { return status; }
    bool hasCredentials() const { return !ssid.empty(); }

private:
    bool loadLegacyCredentials(StorageSystem* storage);
    void refreshStatus();
    void setState(State state, const std::string& message);

private:
    bool initialized = false;
    std::string ssid;
    std::string password;
    Status status;
    unsigned long connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 10000;
    DeviceConfigRepository configRepository;
};
