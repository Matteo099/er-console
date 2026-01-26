#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "BootManager.h"
#include "../ui/UIManager.h"
#include "../devices/InputManager.h"
#include "../devices/audio/AudioManager.h"
#include "../ui/UIManager.h"
#include "../lua/LuaManager.h"


struct TaskManagerStruct
{
    TaskHandle_t bootTaskHandler;
    TaskHandle_t uiTaskHandler;
    TaskHandle_t inputTaskHandler;
    TaskHandle_t audioTaskHandler;
    TaskHandle_t luaTaskHandler;
};

class EscapeOS
{
public:
    void init();
    bool isRunning() { return running; }

    BootManager* getBootManager() { return &bootManager; }
    UIManager* getUIManager() { return &uiManager; }
    InputManager* getInputManager() { return &inputManager; }
    AudioManager* getAudioManager() { return &audioManager; }
    LuaManager* getLuaManager() { return &luaManager; }


    // Eventi di sistema
    static EventGroupHandle_t systemEvents;
    static QueueHandle_t bootQueue;

    // Bit di stato
    static constexpr EventBits_t EVT_BOOT_DONE = (1 << 0);
    static constexpr EventBits_t EVT_UI_READY = (1 << 1);
    static constexpr EventBits_t EVT_LUA_READY = (1 << 2);

private:
    bool running = false;
    TaskManagerStruct tasks;

    BootManager bootManager;
    UIManager uiManager;
    InputManager inputManager;
    AudioManager audioManager;
    LuaManager luaManager;

    void createTasks();
};
