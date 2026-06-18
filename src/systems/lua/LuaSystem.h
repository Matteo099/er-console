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
class GameView;

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
    void restart();

    // State
    bool isReady() const { return ready; }
    bool isScriptLoaded() const { return scriptLoaded; }

    // Lua VM access
    lua_State* getVM() { return L; }

    // API registration (for dependency injection)
    void setGameView(GameView* view) { gameView = view; }
    void setAudioSystem(AudioSystem* sys) { audioSystem = sys; }
    void setInputSystem(InputSystem* sys) { inputSystem = sys; }

private:
    // Lua VM
    lua_State* L = nullptr;

    // Dependencies
    StorageSystem* storage = nullptr;
    GameView* gameView = nullptr;
    AudioSystem* audioSystem = nullptr;
    InputSystem* inputSystem = nullptr;
    uint32_t lastTickMs = 0;

    // State flags
    bool ready = false;
    bool scriptLoaded = false;

    // Internal helpers
    bool createVM();
    void destroyVM();
    void registerBaseLibraries();
    void registerGameAPI();

    void tick(); // one non-blocking execution step
};

