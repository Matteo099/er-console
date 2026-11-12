#include "LuaSystem.h"

#include "log/Logger.h"
#include "scenes/game/GameView.h"
#include <string>
#include <Arduino.h>
#include "bindings/LuaBindingManager.h"
#include "bindings/TimeBindings.h"
#include "bindings/InputBindings.h"
#include "bindings/AudioBindings.h"
#include "bindings/CoreBindings.h"
#include "bindings/LuaConstantsBinding.h"
#include "bindings/GameSceneBindings.h"
#include "LuaAssetLoader.h"

LuaSystem::LuaSystem() {
}

LuaSystem::~LuaSystem() {
    TaskMutex::Guard lock(vmMutex);
    destroyVM();
}

void LuaSystem::init(void* params) {
    TaskMutex::Guard lock(vmMutex);
    Log::info(Log::LUA_TAG, "Initializing LuaSystem");

    if (!storageSystem) {
        Log::error(Log::LUA_TAG, "StorageSystem not provided");
        return;
    }

    if (!createVM()) {
        Log::error(Log::LUA_TAG, "Failed to create Lua VM");
        return;
    }

    registerBaseLibraries();
    registerGameAPI();

    ready = true;
    Log::info(Log::LUA_TAG, "LuaSystem ready");
}

bool LuaSystem::createVM() {
    L = luaL_newstate();
    if (!L)
        return false;

    luaL_openlibs(L);  // base + math + string + table
    return true;
}

void LuaSystem::destroyVM() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }

    ready = false;
    scriptLoaded = false;
}

void LuaSystem::update() {
    TaskMutex::Guard lock(vmMutex);
    if (!ready || !scriptLoaded)
        return;

    tick();
}

bool LuaSystem::loadScript(const char* path) {
    TaskMutex::Guard lock(vmMutex);
    scriptLoaded = false;
    gamePaused = true;
    errorKind = ErrorKind::None;
    lastError.clear();
    if (!storageSystem || !path) {
        errorKind = ErrorKind::Storage;
        lastError = "Storage non disponibile";
        return false;
    }

    // Validate the entire read before compiling. A failed streaming read is not EOF.
    std::string source;
    bool read = false;
    for (unsigned attempt = 0; attempt < 3 && !read; ++attempt) {
        read = storageSystem->readTextFile(path, source, true);
        if (!read && attempt < 2)
            vTaskDelay(pdMS_TO_TICKS(30));
    }
    if (!read) {
        errorKind = ErrorKind::Storage;
        lastError = std::string("Impossibile leggere il file completo: ") + path;
        Log::error(Log::LUA_TAG, "%s", lastError.c_str());
        return false;
    }
    if (source.compare(0, 3, "\xEF\xBB\xBF") == 0)
        source.erase(0, 3);
    if (source.empty()) {
        errorKind = ErrorKind::Syntax;
        lastError = "Il file del gioco e' vuoto";
        return false;
    }

    reset();
    if (!L) {
        errorKind = ErrorKind::Initialization;
        lastError = "Memoria insufficiente per avviare Lua";
        return false;
    }
    auto failed = [&](ErrorKind kind, const char* phase) {
        errorKind = kind;
        const char* message = lua_tostring(L, -1);
        lastError = message ? message : "Errore Lua";
        Log::error(Log::LUA_TAG, "%s (%s): %s", phase, path, lastError.c_str());
        lua_pop(L, 1);
        return false;
    };
    installGameAssetLoaders(L, storageSystem, path);
    if (luaL_loadbuffer(L, source.data(), source.size(), path) != LUA_OK)
        return failed(ErrorKind::Syntax, "Lua load error");
    Log::info(Log::LUA_TAG, "Executing Lua script: %s (%u bytes)", path, unsigned(source.size()));
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        return failed(ErrorKind::Runtime, "Lua runtime error");

    lua_getglobal(L, "init");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            return failed(ErrorKind::Initialization, "Lua init error");
    } else
        lua_pop(L, 1);
    lastTickMs = millis();
    scriptLoaded = true;
    return true;
}

bool LuaSystem::loadMainScript(const char* path) {
    return loadScript(path);
}

void LuaSystem::unloadScript() {
    TaskMutex::Guard lock(vmMutex);
    const bool wasLoaded = scriptLoaded;
    scriptLoaded = false;
    gamePaused = true;
    gameScene = nullptr;
    if (L && wasLoaded) {
        lua_getglobal(L, "exit");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                Log::error(Log::LUA_TAG, "Lua exit error: %s", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        } else
            lua_pop(L, 1);
    }
    if (inputSystem)
        inputSystem->shutdown();
}

void LuaSystem::tick() {
    if (!L)
        return;

    lua_getglobal(L, "update");

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    const uint32_t now = millis();
    float dt = 0.0f;

    if (lastTickMs != 0) {
        dt = static_cast<float>(now - lastTickMs) / 1000.0f;
        constexpr float MAX_DT = 0.25f;
        if (dt > MAX_DT)
            dt = MAX_DT;
    }

    lastTickMs = now;
    TimeBindings::setDelta(dt);
    lua_pushnumber(L, dt);
    lua_pushboolean(L, gamePaused);

    if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
        errorKind = ErrorKind::Runtime;
        lastError = lua_tostring(L, -1) ? lua_tostring(L, -1) : "Errore Lua";
        Log::error(Log::LUA_TAG, "Lua update() error: %s", lastError.c_str());
        lua_pop(L, 1);
        scriptLoaded = false;
    }
}

void LuaSystem::reset() {
    TaskMutex::Guard lock(vmMutex);
    destroyVM();

    if (!createVM())
        return;

    registerBaseLibraries();
    registerGameAPI();
    ready = true;
    lastTickMs = millis();
}

void LuaSystem::registerGameAPI() {
    Log::info(Log::LUA_TAG, "Registering Lua APIs");
    Log::info(Log::LUA_TAG, "storage=%p input=%p audio=%p", storageSystem, inputSystem,
              audioSystem);

    LuaBindingManager manager;

    manager.add(new InputBindings(inputSystem));
    manager.add(new AudioBindings(audioSystem));
    manager.add(new TimeBindings());
    manager.add(new CoreBindings());
    manager.add(new LuaConstantsBinding());
    manager.add(new GameSceneBindings(gameScene));

    manager.registerAll(L);

    Log::info(Log::LUA_TAG, "Lua APIs registered");
}

void LuaSystem::registerBaseLibraries() {
    // Libraries are already opened by luaL_openlibs in createVM
    // Future enhancements could add more specific libraries
}
