#pragma once
#include "esp_log.h"
#include <cstddef>
#include <string>

namespace Log {

    constexpr const char* OS_TAG = "OS";
    constexpr const char* BOOT_TAG = "BOOT";
    constexpr const char* UI_TAG = "UI";
    constexpr const char* STORAGE_TAG = "STORAGE";
    constexpr const char* AUDIO_TAG = "AUDIO";
    constexpr const char* LUA_TAG = "LUA";
    constexpr const char* LUA_SCRIPT_TAG = "LUASCRIPT";
    constexpr const char* GAME_TAG = "GAME";
    constexpr const char* DISPLAY_TAG = "DISPLAY";
    constexpr const char* INPUT_TAG = "INPUT";
    constexpr const char* DRIVER_TAG = "DRIVER";
    constexpr const char* NETWORK_TAG = "NETWORK";
    constexpr const char* SCENE_TAG = "SCENE";
    constexpr const char* LED_TAG = "LED";
    constexpr const char* PORTAL_TAG = "PORTAL";

    enum class Level {
        ERROR,
        WARNING,
        INFO,
        DEBUG
    };

    void init(Level level);

    void error(const char* tag, const char* format, ...);
    void warn(const char* tag, const char* format, ...);
    void info(const char* tag, const char* format, ...);
    void debug(const char* tag, const char* format, ...);

    std::string getRecentLogs(std::size_t maxLines = 200);
    void clearRecentLogs();
}
