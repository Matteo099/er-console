#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

class EscapeOS {
public:
    void init();

    // Eventi di sistema
    static EventGroupHandle_t systemEvents;
    static QueueHandle_t bootQueue;

    // Bit di stato
    static constexpr EventBits_t EVT_BOOT_DONE   = (1 << 0);
    static constexpr EventBits_t EVT_UI_READY    = (1 << 1);
    static constexpr EventBits_t EVT_LUA_READY   = (1 << 2);

private:
    // Task entry point
    static void bootTask(void* param);
    static void uiTask(void* param);
    static void inputTask(void* param);
    static void audioTask(void* param);
    static void luaTask(void* param);

    void createTasks();
};
