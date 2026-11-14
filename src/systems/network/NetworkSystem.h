#pragma once

#include "core/SystemService.h"
#include "services/DeviceConfigRepository.h"
#include <string>
#include "core/TaskMutex.h"
#include "WifiScan.h"

class NetworkSystem : public SystemService {
  public:
    enum class State { Idle, Connecting, Connected, Disconnected, Error };

    struct Status {
        std::string ssid;
        std::string ip;
        State state = State::Idle;
        std::string message;
        int signal = 0;
        bool timedOut = false;
    };

    NetworkSystem();

    void init(void* params = nullptr) override;
    void update() override;

    bool connect();
    void disconnect();
    bool reloadConfiguration();
    void setCredentials(const std::string& newSsid, const std::string& newPassword);

    Status getStatus() const {
        TaskMutex::Guard lock(mutex);
        return status;
    }

    bool hasCredentials() const {
        TaskMutex::Guard lock(mutex);
        return !ssid.empty();
    }

    bool requestScan();

    WifiScanSnapshot getScanSnapshot() const {
        TaskMutex::Guard lock(mutex);
        return scan;
    }

    bool scanSnapshot(uint32_t seenRevision, WifiScanSnapshot& result) const {
        TaskMutex::Guard lock(mutex);
        if (seenRevision == scan.revision)
            return false;
        result = scan;
        return true;
    }

  private:
    void refreshStatus();
    void setState(State state, const std::string& message, bool timedOut = false);
    void updateScan();

  private:
    bool initialized = false;
    std::string ssid;
    std::string password;
    Status status;
    unsigned long connectStartMs = 0;
    static constexpr unsigned long CONNECT_TIMEOUT_MS = 10000;
    DeviceConfigRepository configRepository;
    mutable TaskMutex mutex;
    enum class Command { None, Connect, Disconnect };
    Command command = Command::None;
    WifiScanSnapshot scan;
    bool scanRequested = false, scanActive = false;
};
