#pragma once
#include "esp_log.h"

namespace Log {

    constexpr const char* OS_TAG = "OS";
    constexpr const char* BOOT_TAG = "BOOT";
    constexpr const char* OTHER_TAG = "OTHER";

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

}
