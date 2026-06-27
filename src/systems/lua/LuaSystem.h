#pragma once

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
#include "systems/audio/AudioSystem.h"
#include "systems/input/InputSystem.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

// Forward declaration
class GameScene;

class LuaSystem : public SystemService
{
public:
    LuaSystem();
    ~LuaSystem();

    // SystemService
    void init(void* params = nullptr) override;
    void update() override;

    // Script control
    bool loadScript(const char* path);
    bool loadMainScript(const char* path);
    void unloadScript();
    void reset();

    // Game control
    void pause() { gamePaused = true; }
    void resume() { gamePaused = false; }

    // State
    bool isReady() const { return ready; }
    bool isScriptLoaded() const { return scriptLoaded; }

    // Lua VM access
    lua_State* getVM() { return L; }

    // API registration (for dependency injection)
    void setGameScene(GameScene* scene) { gameScene = scene; }
    void setStorageSystem(StorageSystem* sys) { storageSystem = sys; }
    void setAudioSystem(AudioSystem* sys) { audioSystem = sys; }
    void setInputSystem(InputSystem* sys) { inputSystem = sys; }

private:
    // Lua VM
    lua_State* L = nullptr;

    // Dependencies
    GameScene* gameScene = nullptr;
    StorageSystem* storageSystem = nullptr;
    AudioSystem* audioSystem = nullptr;
    InputSystem* inputSystem = nullptr;
    uint32_t lastTickMs = 0;

    // State flags
    bool ready = false;
    bool scriptLoaded = false;
    bool gamePaused = true;

    // Internal helpers
    bool createVM();
    void destroyVM();
    void registerBaseLibraries();
    void registerGameAPI();

    void tick(); // one non-blocking execution step
};

