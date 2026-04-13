#include "Logger.h"

#include <Arduino.h>
#include <cstdarg>
#include <deque>
#include <string>

static esp_log_level_t currentLevel = ESP_LOG_INFO;
static std::deque<std::string> recentLogs;
static constexpr std::size_t MAX_RECENT_LOG_LINES = 400;

namespace
{
    static const char* levelToString(esp_log_level_t level)
    {
        switch (level)
        {
            case ESP_LOG_ERROR: return "ERROR";
            case ESP_LOG_WARN: return "WARN";
            case ESP_LOG_INFO: return "INFO";
            case ESP_LOG_DEBUG: return "DEBUG";
            default: return "LOG";
        }
    }
}

void Log::init(Level level) {
    Serial.begin(115200);
    
    switch (level) {
        case Level::ERROR:   currentLevel = ESP_LOG_ERROR; break;
        case Level::WARNING: currentLevel = ESP_LOG_WARN;  break;
        case Level::INFO:    currentLevel = ESP_LOG_INFO;  break;
        case Level::DEBUG:   currentLevel = ESP_LOG_DEBUG; break;
    }

    esp_log_level_set("*", currentLevel);
}

// static void logInternal(esp_log_level_t level, const char* tag, const char* format, va_list args) {
//     esp_log_writev(level, tag, format, args);
// }
static void logInternal(esp_log_level_t level, const char* tag, const char* format, va_list args) {
    if (level > currentLevel)
        return;

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);

    char line[320];
    snprintf(line, sizeof(line), "[%lu][%s][%s] %s", millis(), levelToString(level), tag, buffer);
    Serial.println(line);

    recentLogs.push_back(std::string(line));
    if (recentLogs.size() > MAX_RECENT_LOG_LINES)
        recentLogs.pop_front();
}


void Log::error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(ESP_LOG_ERROR, tag, format, args);
    va_end(args);
}

void Log::warn(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(ESP_LOG_WARN, tag, format, args);
    va_end(args);
}

void Log::info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(ESP_LOG_INFO, tag, format, args);
    va_end(args);
}

void Log::debug(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(ESP_LOG_DEBUG, tag, format, args);
    va_end(args);
}

std::string Log::getRecentLogs(std::size_t maxLines)
{
    if (recentLogs.empty())
        return std::string();

    std::size_t count = maxLines == 0 || maxLines > recentLogs.size() ? recentLogs.size() : maxLines;
    std::size_t start = recentLogs.size() - count;

    std::string result;
    for (std::size_t i = start; i < recentLogs.size(); ++i)
    {
        result += recentLogs[i];
        result += '\n';
    }

    return result;
}

void Log::clearRecentLogs()
{
    recentLogs.clear();
}
