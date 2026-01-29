#include "LuaSystem.h"

#include "log/Logger.h"

LuaSystem::LuaSystem() {}

LuaSystem::~LuaSystem()
{
    destroyVM();
}

void LuaSystem::init(void* params)
{
    Log::info(Log::LUA_TAG, "Initializing LuaSystem");

    storage = static_cast<StorageSystem*>(params);
    if (!storage)
    {
        Log::error(Log::LUA_TAG, "StorageSystem not provided");
        return;
    }

    if (!createVM())
    {
        Log::error(Log::LUA_TAG, "Failed to create Lua VM");
        return;
    }

    registerBaseLibraries();

    // Load entry point
    if (!loadMainScript("/scripts/main.lua"))
    {
        Log::error(Log::LUA_TAG, "Failed to load main.lua");
        return;
    }

    ready = true;
    Log::info(Log::LUA_TAG, "LuaSystem ready");
}

bool LuaSystem::createVM()
{
    L = luaL_newstate();
    if (!L)
        return false;

    luaL_openlibs(L); // base + math + string + table
    return true;
}

void LuaSystem::destroyVM()
{
    if (L)
    {
        lua_close(L);
        L = nullptr;
    }

    ready = false;
    scriptLoaded = false;
}


void LuaSystem::update()
{
    if (!ready || !scriptLoaded)
        return;

    tick();
}

bool LuaSystem::loadMainScript(const char* path)
{
    if (!L || !storage)
        return false;

    // std::string script;
    // if (!storage->readTextFile(path, script))
    // {
    //     Log::error(Log::LUA_TAG, "Cannot read script: %s", path);
    //     return false;
    // }

    // if (luaL_loadstring(L, script.c_str()) != LUA_OK)
    // {
    //     Log::error(Log::LUA_TAG, "Lua load error: %s", lua_tostring(L, -1));
    //     return false;
    // }

    // if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    // {
    //     Log::error(Log::LUA_TAG, "Lua runtime error: %s", lua_tostring(L, -1));
    //     return false;
    // }

    scriptLoaded = true;
    return true;
}

void LuaSystem::tick()
{
    // For now: nothing
    // Later:
    // - resume coroutine
    // - process queued events
    // - execute timed callbacks
}

void LuaSystem::restart()
{
    destroyVM();
    createVM();
    registerBaseLibraries();
    loadMainScript("/scripts/main.lua");
}

void LuaSystem::registerBaseLibraries()
{
    // Future:
    // register ui.*
    // register audio.*
    // register game.*
    // register timer.*
}
