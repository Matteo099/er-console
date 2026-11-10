#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <unordered_map>
#include "SystemService.h"

enum SystemEventBits {
    EVT_RAINBOWLED_INIT_REQ = (1 << 0),
    EVT_RAINBOWLED_READY    = (1 << 1),

    EVT_UI_INIT_REQ         = (1 << 2),
    EVT_UI_READY            = (1 << 3),
   
    EVT_STORAGE_INIT_REQ    = (1 << 4),
    EVT_STORAGE_READY       = (1 << 5),

    EVT_NETWORK_INIT_REQ    = (1 << 6),
    EVT_NETWORK_READY       = (1 << 7),
   
    EVT_AUDIO_INIT_REQ      = (1 << 8),
    EVT_AUDIO_READY         = (1 << 9),
   
    EVT_INPUT_INIT_REQ      = (1 << 10),
    EVT_INPUT_READY         = (1 << 11),
   
    EVT_LUA_INIT_REQ        = (1 << 12),
    EVT_LUA_READY           = (1 << 13),
   
    EVT_BOOT_DONE           = (1 << 14),
};

class SystemEventBus
{
public:
    // Service identifiers
    static constexpr int SERVICE_LUA = 1;
    static constexpr int SERVICE_AUDIO = 2;
    static constexpr int SERVICE_STORAGE = 3;
    static constexpr int SERVICE_UI = 4;
    static constexpr int SERVICE_INPUT = 5;
    static constexpr int SERVICE_NETWORK = 6;
    static constexpr int SERVICE_PORTAL = 7;

    // Singleton instance
    static SystemEventBus& instance();

    // Service registration
    void registerService(int serviceId, SystemService* service);
    SystemService* getService(int serviceId);

    // Events
    static EventGroupHandle_t systemEvents;
    static QueueHandle_t bootQueue;
    static QueueHandle_t audioQueue;

    static void init();

private:
    SystemEventBus() = default;
    std::unordered_map<int, SystemService*> services;
};
