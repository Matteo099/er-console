#pragma once
#include "components/StatusBadge.h"

// One controller updates the current screen; each screen owns its floating view.
class DeviceStatusBar
{
public:
    enum class StorageState { Unavailable, Ready, Degraded, Error };
    enum class WiFiState { Disconnected, Connecting, Connected, AccessPoint, Error };
    static constexpr int Width = 92;

    void attachTo(lv_obj_t *screen);
    void update(bool force = false);
    void preview(StorageState sd, WiFiState wifi);

private:
    void render(StorageState sd, WiFiState wifi);
    lv_obj_t *root = nullptr;
    StatusBadge::Parts sdBadge, wifiBadge;
    uint32_t lastRefresh = 0;
    bool previewing = false;
};
