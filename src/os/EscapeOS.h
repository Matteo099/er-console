#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "os/BootManager.h"
#include "systems/ui/UIManager.h"
#include "systems/input/InputSystem.h"
#include "systems/audio/AudioSystem.h"
#include "systems/lua/LuaSystem.h"

class EscapeOS
{
public:
    void init();
    bool isRunning() { return running; }

    BootManager* getBootManager() { return &bootManager; }
    UIManager* getUIManager() { return &uiManager; }
    InputSystem* getInputSystem() { return &inputSystem; }
    AudioSystem* getAudioSystem() { return &audioSystem; }
    LuaSystem* getLuaSystem() { return &luaSystem; }

private:
    bool running = false;

    BootManager bootManager;
    UIManager uiManager;
    InputSystem inputSystem;
    AudioSystem audioSystem;
    LuaSystem luaSystem;

    void createTasks();
};
