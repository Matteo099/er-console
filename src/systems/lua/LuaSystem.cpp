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

LuaSystem::LuaSystem() {}

LuaSystem::~LuaSystem()
{
    destroyVM();
}

void LuaSystem::init(void *params)
{
    Log::info(Log::LUA_TAG, "Initializing LuaSystem");

    if (!storageSystem)
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
    reset(); // reset state before loading new script

    if (!L || !storageSystem)
        return false;

    IFile *file = storageSystem->open(path);
    if (!file)
    {
        Log::error(Log::LUA_TAG, "Cannot open script: %s", path);
        return false;
    }


    // Load Lua chunk via streaming reader
    if (lua_load(L, luaFileReader, file, path, nullptr) != LUA_OK)
    {
        Log::error(Log::LUA_TAG, "Lua load error (%s): %s", path, lua_tostring(L, -1));
        lua_pop(L, 1);
        file->close();
        delete file;
        return false;
    }

    // Execute script
    Log::info(Log::LUA_TAG, "Executing Lua script: %s", path);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        Log::error(Log::LUA_TAG, "Lua runtime error (%s): %s", path, lua_tostring(L, -1));
        lua_pop(L, 1);
        file->close();
        delete file;
        return false;
    }

    // Debug: Check if 'system' table is available
    // lua_getglobal(L, "system");
    // Log::debug(Log::LUA_TAG, "system type = %s", lua_typename(L, lua_type(L, -1)));
    // lua_pop(L, 1);

    // Optional init()
    lua_getglobal(L, "init");
    Log::debug(Log::LUA_TAG, "init type = %s", lua_typename(L, lua_type(L, -1)));
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            Log::error(Log::LUA_TAG, "Lua init() error: %s", lua_tostring(L, -1));
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
    gamePaused = true;

    if (!L)
        return;

    // Call Lua exit() if it exists
    lua_getglobal(L, "exit");

    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            Log::error(Log::LUA_TAG, "Lua exit() error: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1); // remove non-function value
    }

    inputSystem->shutdown();
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
    lua_pushboolean(L, gamePaused);

    if (lua_pcall(L, 2, 0, 0) != LUA_OK)
    {
        Log::error(Log::LUA_TAG, "Lua update() error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        scriptLoaded = false;
    }
}

void LuaSystem::reset()
{
    destroyVM();

    if (!createVM())
        return;

    registerBaseLibraries();
    registerGameAPI();
    ready = true;
    lastTickMs = millis();
}

void LuaSystem::registerGameAPI()
{
    Log::info(Log::LUA_TAG, "Registering Lua APIs");
    Log::info(Log::LUA_TAG, "storage=%p input=%p audio=%p", storageSystem, inputSystem, audioSystem);

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

void LuaSystem::registerBaseLibraries()
{
    // Libraries are already opened by luaL_openlibs in createVM
    // Future enhancements could add more specific libraries
}
