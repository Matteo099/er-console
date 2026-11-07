#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>

struct WifiScanEntry
{
    WifiScanEntry() = default;
    WifiScanEntry(std::string name, int rssi, bool secured, std::string auth)
        : ssid(std::move(name)), signal(rssi), secure(secured), authMode(std::move(auth)) {}
    std::string ssid;
    int signal = 0;
    bool secure = false;
    std::string authMode;
};

enum class WifiScanState { Idle, Scanning, Ready, Error, Disabled, Busy };
struct WifiScanSnapshot
{
    WifiScanState state = WifiScanState::Idle;
    std::vector<WifiScanEntry> networks;
    uint32_t revision = 1;
};

namespace WifiScan
{
    inline unsigned signalBars(int rssi)
    {
        if (rssi >= 0 || rssi < -110) return 0;
        return rssi >= -55 ? 4 : rssi >= -67 ? 3 : rssi >= -80 ? 2 : 1;
    }
    inline void normalize(std::vector<WifiScanEntry> &entries)
    {
        std::sort(entries.begin(), entries.end(), [](const WifiScanEntry &a, const WifiScanEntry &b) {
            return a.signal != b.signal ? a.signal > b.signal : a.ssid < b.ssid;
        });
        std::vector<WifiScanEntry> visible;
        for (const auto &entry : entries)
        {
            if (entry.ssid.empty()) continue;
            const bool duplicate = std::any_of(visible.begin(),visible.end(),[&](const WifiScanEntry &other) {
                return entry.ssid == other.ssid && entry.secure == other.secure;
            });
            if (!duplicate) visible.push_back(entry);
            if (visible.size() == 24) break;
        }
        entries.swap(visible);
    }
}
