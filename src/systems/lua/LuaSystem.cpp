#include "LuaSystem.h"

#include "log/Logger.h"
#include <string>

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

    // Load entry point
    // if (!loadMainScript("/scripts/main.lua"))
    // {
    //     Log::error(Log::LUA_TAG, "Failed to load main.lua");
    //     return;
    // }

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

static const char* luaFileReader(lua_State* L, void* data, size_t* size)
{
    IFile* file = static_cast<IFile*>(data);

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
    return reinterpret_cast<const char*>(buffer);
}

bool LuaSystem::loadMainScript(const char* path)
{
    if (!L || !storage)
        return false;

    IFile* file = storage->open(path);
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

    scriptLoaded = true;
    return true;
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

    // dt in seconds (for now fixed)
    lua_pushnumber(L, 0.02); // 20 ms tick

    if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    {
        Log::error(Log::LUA_TAG, "Lua update() error: %s", lua_tostring(L, -1));
        lua_pop(L, 1);

        // optional: disable script on fatal error
        scriptLoaded = false;
    }
}

void LuaSystem::restart()
{
    destroyVM();

    if (!createVM())
        return;

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
