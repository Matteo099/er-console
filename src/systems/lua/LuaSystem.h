#pragma once

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
#include "systems/audio/AudioSystem.h"
#include "systems/input/InputSystem.h"
#include "core/TaskMutex.h"

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
    enum class ErrorKind { None, Storage, Syntax, Runtime, Initialization };
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
    void pause() { TaskMutex::Guard lock(vmMutex); gamePaused = true; }
    void resume() { TaskMutex::Guard lock(vmMutex); gamePaused = false; lastTickMs = 0; }

    // State
    bool isReady() const { TaskMutex::Guard lock(vmMutex); return ready; }
    bool isScriptLoaded() const { TaskMutex::Guard lock(vmMutex); return scriptLoaded; }
    ErrorKind getErrorKind() const { TaskMutex::Guard lock(vmMutex); return errorKind; }
    std::string getLastError() const { TaskMutex::Guard lock(vmMutex); return lastError; }
    bool isBoundTo(const GameScene *scene) const { TaskMutex::Guard lock(vmMutex); return gameScene == scene; }

    // Lua VM access
    lua_State* getVM() { return L; }

    // API registration (for dependency injection)
    void setGameScene(GameScene* scene) { TaskMutex::Guard lock(vmMutex); gameScene = scene; }
    void setStorageSystem(StorageSystem* sys) { storageSystem = sys; }
    void setAudioSystem(AudioSystem* sys) { audioSystem = sys; }
    void setInputSystem(InputSystem* sys) { inputSystem = sys; }

private:
    mutable TaskMutex vmMutex;
    ErrorKind errorKind = ErrorKind::None;
    std::string lastError;
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

