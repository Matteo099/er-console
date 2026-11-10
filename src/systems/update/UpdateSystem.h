#pragma once
#include "config/Console.h"
#include "UpdateTypes.h"
#include "core/TaskMutex.h"
#include <freertos/task.h>
#include <atomic>
class UpdateInput;

class UpdateSystem
{
public:
    static UpdateSystem &instance();
    UpdateSnapshot snapshot() const;
    uint32_t revision() const
    {
        TaskMutex::Guard lock(mutex);
        return status.revision;
    }
    bool check();
    bool install();
    void cancel();
    bool saveToken(const std::string &value);
    void beginBootVerification();
    void uiHeartbeat();
    static bool flashInProgress() { return flashing.load(); }
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    UpdateError validateInactiveForDiagnostics(UpdateInput &input, const UpdateRelease &release);
#endif
private:
    mutable TaskMutex mutex;
    UpdateSnapshot status;
    std::atomic<bool> cancellation{false};
    std::atomic<uint32_t> heartbeat{0};
    static std::atomic<bool> flashing;
    bool working = false, installRequested = false;
    uint32_t startedAt = 0;
    bool request(bool installing);
    bool stopped() const;
    void fail(UpdateError error);
    void progress(UpdateState state, size_t downloaded, size_t written);
    static void worker(void *context);
    static void verifier(void *context);
    void run();
};
