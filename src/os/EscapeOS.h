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
#include "systems/network/NetworkSystem.h"
#include "systems/portal/PortalSystem.h"

class EscapeOS {
  public:
    void init();

    bool isRunning() {
        return running;
    }

    BootManager* getBootManager() {
        return &bootManager;
    }

    UIManager* getUIManager() {
        return &uiManager;
    }

    StorageSystem* getStorageSystem() {
        return &storageSystem;
    }

    InputSystem* getInputSystem() {
        return &inputSystem;
    }

    AudioSystem* getAudioSystem() {
        return &audioSystem;
    }

    LuaSystem* getLuaSystem() {
        return &luaSystem;
    }

    RainbowPixel* getRainbowPixel() {
        return &rainbowPixel;
    }

    NetworkSystem* getNetworkSystem() {
        return &networkSystem;
    }

    PortalSystem* getPortalSystem() {
        return &portalSystem;
    }

  private:
    bool running = false;

    BootManager bootManager;
    RainbowPixel rainbowPixel;
    UIManager uiManager;
    StorageSystem storageSystem;
    InputSystem inputSystem;
    AudioSystem audioSystem;
    LuaSystem luaSystem;
    NetworkSystem networkSystem;
    PortalSystem portalSystem;

    void createTasks();
};
