#pragma once

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
// #include <lua.hpp>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class LuaSystem : public SystemService
{
public:
    LuaSystem();
    ~LuaSystem();

    // SystemService
    void init(void* params = nullptr) override;
    void update() override;

    // Script control
    bool loadMainScript(const char* path);
    void restart();

    // State
    bool isReady() const { return ready; }

private:
    // Lua VM
    lua_State* L = nullptr;

    // Dependencies
    StorageSystem* storage = nullptr;

    // State flags
    bool ready = false;
    bool scriptLoaded = false;

    // Internal helpers
    bool createVM();
    void destroyVM();
    void registerBaseLibraries();

    void tick(); // one non-blocking execution step
};
