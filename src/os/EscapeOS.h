#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "os/BootManager.h"
#include "systems/ui/UIManager.h"
#include "systems/storage/StorageSystem.h"
#include "systems/input/InputSystem.h"
#include "systems/audio/AudioSystem.h"
#include "systems/lua/LuaSystem.h"
#include "systems/rainbowpixel/RainbowPixel.h"

class EscapeOS
{
public:
    void init();
    bool isRunning() { return running; }

    BootManager* getBootManager() { return &bootManager; }
    UIManager* getUIManager() { return &uiManager; }
    StorageSystem* getStorageSystem() { return &storageSystem; }
    InputSystem* getInputSystem() { return &inputSystem; }
    AudioSystem* getAudioSystem() { return &audioSystem; }
    LuaSystem* getLuaSystem() { return &luaSystem; }
    RainbowPixel* getRainbowPixel() { return &rainbowPixel; }

private:
    bool running = false;

    BootManager bootManager;
    RainbowPixel rainbowPixel;
    UIManager uiManager;
    StorageSystem storageSystem;
    InputSystem inputSystem;
    AudioSystem audioSystem;
    LuaSystem luaSystem;

    void createTasks();
};
