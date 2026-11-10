#include "StorageRecoverySystem.h"

#include <esp_system.h>

#include "log/Logger.h"

namespace
{
    constexpr uint32_t POWER_OFF_DELAY_MS = 1000;
    constexpr uint32_t POWER_ON_DELAY_MS = 1500;
    constexpr uint32_t SOFT_RECOVERY_DELAY_MS = 500;
}

StorageRecoverySystem::StorageRecoverySystem(
    SDCardDriver *driver,
    uint8_t relayPin,
    bool relayActiveHigh)
    : driver(driver),
      relayPin(relayPin),
      relayActiveHigh(relayActiveHigh)
{
}

void StorageRecoverySystem::init()
{
    if(initialized)
        return;

    pinMode(relayPin, OUTPUT);

    //
    // Keep storage powered by default
    //
    digitalWrite(
        relayPin,
        relayActiveHigh ? LOW : HIGH);

    printLastFailure();
    store.clear();
    initialized = true;
}

StorageRecoveryState StorageRecoverySystem::lastFailure()
{
    return store.load();
}

void StorageRecoverySystem::printLastFailure()
{
    auto state = lastFailure();

    if (state.reason == StorageFailureReason::None)
    {
        Log::info(Log::STORAGE_TAG, "No previous storage failures recorded");
        return;
    }

    Log::info(
        Log::STORAGE_TAG,
        "Last storage failure: %s (bootCount=%u, failureCount=%u, timestamp=%u)",
        reasonToString(state.reason),
        state.bootCount,
        state.failureCount,
        state.timestamp);
}

bool StorageRecoverySystem::recover(StorageFailureReason reason, bool restart)
{
    init();
    
    store.save(reason);

    Log::error(
        Log::STORAGE_TAG,
        "Storage recovery requested (%s)",
        reasonToString(reason));

    //
    // Attempt soft recovery first
    //
    if (softRecover())
    {
        Log::info(Log::STORAGE_TAG, "Soft recovery completed");
        return true;
    }
    else
    {
        Log::warn(Log::STORAGE_TAG, "Soft recovery failed");
    }

    //
    // Attempt hardware recovery
    //
    if (powerCycleStorage() && softRecover())
    {
        Log::info(Log::STORAGE_TAG, "Hardware recovery completed");
        return true;
    }
    else
    {
        Log::warn(Log::STORAGE_TAG, "Hardware recovery failed");
    }

    if (restart)
    {
        Log::info(Log::STORAGE_TAG, "Restarting ESP32");
        delay(250);
        esp_restart();
    }

    Log::error(Log::STORAGE_TAG, "Storage recovery failed");
    return false;
}

bool StorageRecoverySystem::softRecover()
{
    Log::warn(Log::STORAGE_TAG, "Attempting soft storage recovery");

    for (int i = 0; i < 3; ++i)
    {
        if (driver->init())
            return true;

        auto msg = "Attempt soft recovery " + std::to_string(i + 1) + " failed";
        Log::warn(Log::STORAGE_TAG, msg.c_str());

        delay(SOFT_RECOVERY_DELAY_MS);
    }

    return false;
}

bool StorageRecoverySystem::powerCycleStorage()
{
    Log::warn(
        Log::STORAGE_TAG,
        "Power cycling storage");

    //
    // Disable storage power
    //
    digitalWrite(
        relayPin,
        relayActiveHigh ? HIGH : LOW);

    delay(POWER_OFF_DELAY_MS);

    //
    // Re-enable storage power
    //
    digitalWrite(
        relayPin,
        relayActiveHigh ? LOW : HIGH);

    delay(POWER_ON_DELAY_MS);

    Log::info(
        Log::STORAGE_TAG,
        "Storage power restored");

    return true;
}

const char *StorageRecoverySystem::reasonToString(
    StorageFailureReason reason)
{
    switch (reason)
    {
    case StorageFailureReason::MountFailed:
        return "MountFailed";

    case StorageFailureReason::ConsecutiveIOFailures:
        return "ConsecutiveIOFailures";

    case StorageFailureReason::MissingCriticalFiles:
        return "MissingCriticalFiles";

    case StorageFailureReason::CorruptionDetected:
        return "CorruptionDetected";

    default:
        return "Unknown";
    }
}
