#include "Logger.h"

#include <Arduino.h>
#include <cstdarg>

static esp_log_level_t currentLevel = ESP_LOG_INFO;

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
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.printf("[%lu][%s] %s\n", millis(), tag, buffer);
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
