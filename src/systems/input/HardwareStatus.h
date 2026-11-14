#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class HardwareIssueKind {
    Configuration,
    Unsupported,
    MissingI2C,
    MissingRFID,
    MissingIdentity,
    IdentityShort,
    IdentityUnstable,
    WrongRoom,
    Initialization
};

struct HardwareIssue {
    HardwareIssueKind kind;
    std::string component;
};

enum class HardwareState { Idle, Checking, Ready, Fault };

struct HardwareStatus {
    HardwareState state = HardwareState::Idle;
    std::vector<HardwareIssue> issues;
};
