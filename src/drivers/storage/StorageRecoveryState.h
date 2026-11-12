#pragma once

#include <Arduino.h>

enum class StorageFailureReason : uint8_t {
    None = 0,
    MountFailed,
    ConsecutiveIOFailures,
    MissingCriticalFiles,
    CorruptionDetected
};

struct StorageRecoveryState {
    StorageFailureReason reason;
    uint32_t bootCount;
    uint32_t failureCount;
    uint32_t timestamp;
};