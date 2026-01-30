#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

enum SystemEventBits {
    EVT_RAINBOWLED_INIT_REQ = (1 << 0),
    EVT_RAINBOWLED_READY    = (1 << 1),

    EVT_UI_INIT_REQ         = (1 << 2),
    EVT_UI_READY            = (1 << 3),
   
    EVT_STORAGE_INIT_REQ    = (1 << 4),
    EVT_STORAGE_READY       = (1 << 5),
   
    EVT_AUDIO_INIT_REQ      = (1 << 6),
    EVT_AUDIO_READY         = (1 << 7),
   
    EVT_INPUT_INIT_REQ      = (1 << 8),
    EVT_INPUT_READY         = (1 << 9),
   
    EVT_LUA_INIT_REQ        = (1 << 10),
    EVT_LUA_READY           = (1 << 11),
   
    EVT_BOOT_DONE           = (1 << 12),
};

class SystemEventBus
{
public:
    // Eventi di sistema
    static EventGroupHandle_t systemEvents;
    static QueueHandle_t bootQueue;
    static QueueHandle_t audioQueue;

    static void init();
};