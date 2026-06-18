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

LuaSystem::LuaSystem() {}

LuaSystem::~LuaSystem()
{
    destroyVM();
}

void LuaSystem::init(void *params)
{
    Log::info(Log::LUA_TAG, "Initializing LuaSystem");

    storage = static_cast<StorageSystem *>(params);
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
    registerGameAPI();

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

static const char *luaFileReader(lua_State *L, void *data, size_t *size)
{
    IFile *file = static_cast<IFile *>(data);

    static uint8_t buffer[512]; // small, safe chunk
    if (!file->available())
    {
        *size = 0;
        return nullptr;
    }

    int read = file->read(buffer, sizeof(buffer));
    if (read <= 0)
    {
        *size = 0;
        return nullptr;
    }

    *size = static_cast<size_t>(read);
    return reinterpret_cast<const char *>(buffer);
}

bool LuaSystem::loadScript(const char *path)
{
    if (!L || !storage)
        return false;

    IFile *file = storage->open(path);
    if (!file)
    {
        Log::error(Log::LUA_TAG, "Cannot open script: %s", path);
        return false;
    }

    // Load Lua chunk via streaming reader
    if (lua_load(L, luaFileReader, file, path, nullptr) != LUA_OK)
    {
        Log::error(
            Log::LUA_TAG,
            "Lua load error (%s): %s",
            path,
            lua_tostring(L, -1));

        lua_pop(L, 1);
        file->close();
        delete file;
        return false;
    }

    // Execute script
    Log::info(Log::LUA_TAG, "Executing Lua script: %s", path);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        Log::error(
            Log::LUA_TAG,
            "Lua runtime error (%s): %s",
            path,
            lua_tostring(L, -1));

        lua_pop(L, 1);
        file->close();
        delete file;
        return false;
    }

    // Optional init()
    lua_getglobal(L, "init");
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            Log::error(
                Log::LUA_TAG,
                "Lua init() error: %s",
                lua_tostring(L, -1));

            lua_pop(L, 1);
            file->close();
            delete file;
            return false;
        }
    }
    else
    {
        lua_pop(L, 1);
    }

    file->close();
    delete file;

    lastTickMs = millis();
    scriptLoaded = true;
    return true;
}

bool LuaSystem::loadMainScript(const char *path)
{
    return loadScript(path);
}

void LuaSystem::unloadScript()
{
    scriptLoaded = false;
}

void LuaSystem::tick()
{
    if (!L)
        return;

    lua_getglobal(L, "update");

    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 1);
        return;
    }

    const uint32_t now = millis();
    float dt = 0.0f;

    if (lastTickMs != 0)
    {
        dt = static_cast<float>(now - lastTickMs) / 1000.0f;
        constexpr float MAX_DT = 0.25f;
        if (dt > MAX_DT)
            dt = MAX_DT;
    }

    lastTickMs = now;
    TimeBindings::setDelta(dt);
    lua_pushnumber(L, dt);

    if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    {
        Log::error(Log::LUA_TAG, "Lua update() error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        scriptLoaded = false;
    }
}

void LuaSystem::restart()
{
    destroyVM();

    if (!createVM())
        return;

    registerBaseLibraries();
    registerGameAPI();
    lastTickMs = millis();
    loadMainScript("/scripts/main.lua");
}

void LuaSystem::registerGameAPI()
{
    LuaBindingManager manager;

    manager.add(new InputBindings(inputSystem));
    manager.add(new AudioBindings(audioSystem));
    manager.add(new TimeBindings());
    manager.add(new CoreBindings());

    manager.registerAll(L);
}

void LuaSystem::registerBaseLibraries()
{
    // Libraries are already opened by luaL_openlibs in createVM
    // Future enhancements could add more specific libraries
}
