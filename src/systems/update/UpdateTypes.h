#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

enum class UpdateState : uint8_t {
    Idle,
    Checking,
    UpToDate,
    Available,
    Downloading,
    Verifying,
    Installing,
    Success,
    Error,
    Cancelled
};
enum class UpdateError : uint8_t {
    None,
    Disabled,
    Network,
    Clock,
    NotFound,
    RateLimit,
    Http,
    Metadata,
    Incompatible,
    Size,
    Integrity,
    Image,
    Flash,
    Memory,
    Timeout,
    Cancelled,
    GameActive,
    BootPending,
    Persistence,
    RollbackUnavailable,
    Interrupted,
    Busy
};

struct ReleaseAsset {
    uint64_t id = 0;
    size_t size = 0;
    std::string name, digest;
};

struct UpdateRelease {
    std::string version, title, notes, published, sha256, filename;
    uint64_t manifestId = 0, firmwareId = 0;
    size_t size = 0;
    bool notesTruncated = false;
    std::vector<ReleaseAsset> assets;
};

struct UpdateSnapshot {
    UpdateState state = UpdateState::Idle;
    UpdateError error = UpdateError::None;
    UpdateRelease release;
    size_t downloaded = 0, written = 0, total = 0;
    uint32_t revision = 0;
    bool cancelling = false, confirmedUpdate = false;

    bool busy() const {
        return state == UpdateState::Checking || state == UpdateState::Downloading ||
               state == UpdateState::Verifying || state == UpdateState::Installing ||
               state == UpdateState::Success;
    }
};
