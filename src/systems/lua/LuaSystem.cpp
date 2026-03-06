#include "LuaSystem.h"

#include "log/Logger.h"
#include "scenes/game/GameView.h"
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

bool LuaSystem::loadScript(const char* path)
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

bool LuaSystem::loadMainScript(const char* path)
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
    registerGameAPI();
    loadMainScript("/scripts/main.lua");
}

/* ================= LUA API BINDINGS ================= */

// Lua: display.clear()
static int lua_display_clear(lua_State* L)
{
    // Access gameView through registry
    lua_getfield(L, LUA_REGISTRYINDEX, "gameView_ptr");
    GameView* gameView = (GameView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (gameView)
        gameView->clearDisplay();

    return 0;
}

// Lua: display.text(x, y, text, [fontSize], [color])
static int lua_display_text(lua_State* L)
{
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char* text = luaL_checkstring(L, 3);
    int fontSize = luaL_optinteger(L, 4, 16);
    uint32_t color = (uint32_t)luaL_optinteger(L, 5, 0xFFFFFF);

    lua_getfield(L, LUA_REGISTRYINDEX, "gameView_ptr");
    GameView* gameView = (GameView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (gameView)
        gameView->drawText(x, y, text, fontSize, color);

    return 0;
}

// Lua: display.rect(x, y, width, height, [color], [fill])
static int lua_display_rect(lua_State* L)
{
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_optinteger(L, 5, 0xFFFFFF);
    bool fill = lua_toboolean(L, 6);

    lua_getfield(L, LUA_REGISTRYINDEX, "gameView_ptr");
    GameView* gameView = (GameView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (gameView)
        gameView->drawRect(x, y, width, height, color, fill);

    return 0;
}

// Lua: display.line(x1, y1, x2, y2, [color])
static int lua_display_line(lua_State* L)
{
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    uint32_t color = (uint32_t)luaL_optinteger(L, 5, 0xFFFFFF);

    lua_getfield(L, LUA_REGISTRYINDEX, "gameView_ptr");
    GameView* gameView = (GameView*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (gameView)
        gameView->drawLine(x1, y1, x2, y2, color);

    return 0;
}

void LuaSystem::registerGameAPI()
{
    if (!L)
        return;

    // Register gameView pointer in registry (used by Lua functions)
    lua_pushlightuserdata(L, (void*)gameView);
    lua_setfield(L, LUA_REGISTRYINDEX, "gameView_ptr");

    // Create display table
    lua_newtable(L);

    lua_pushcfunction(L, lua_display_clear);
    lua_setfield(L, -2, "clear");

    lua_pushcfunction(L, lua_display_text);
    lua_setfield(L, -2, "text");

    lua_pushcfunction(L, lua_display_rect);
    lua_setfield(L, -2, "rect");

    lua_pushcfunction(L, lua_display_line);
    lua_setfield(L, -2, "line");

    lua_setglobal(L, "display");

    Log::info(Log::LUA_TAG, "Registered display API");
}

void LuaSystem::registerBaseLibraries()
{
    // Libraries are already opened by luaL_openlibs in createVM
    // Future enhancements could add more specific libraries
}
