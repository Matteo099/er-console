#include "LuaAssetLoader.h"
#include "systems/storage/StorageSystem.h"
#include "services/GameAssetCache.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace {
std::string resolve(lua_State* L, const char* name) {
    std::string path = name;
    if (path.compare(0, 4, "/sd/") == 0)
        path.erase(0, 3);
    if (path.empty() || (path[0] != '/' && path.compare(0, 2, "S:") != 0))
        path = std::string(lua_tostring(L, lua_upvalueindex(2))) + "/" + path;
    return GameAssetCache::normalize(path.c_str());
}

int compile(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* mode = luaL_optstring(L, 2, nullptr);
    const bool hasEnvironment = lua_gettop(L) >= 3;
    auto* storage = static_cast<StorageSystem*>(lua_touserdata(L, lua_upvalueindex(1)));
    const auto path = resolve(L, name);
    std::string source;
    if (path.empty() || !storage->readTextFile(path.c_str(), source)) {
        lua_pushnil(L);
        lua_pushfstring(L, "Cannot read script: %s", name);
        return 2;
    }
    if (source.compare(0, 3, "\xEF\xBB\xBF") == 0)
        source.erase(0, 3);
    if (luaL_loadbufferx(L, source.data(), source.size(), path.c_str(), mode) != LUA_OK) {
        lua_pushnil(L);
        lua_insert(L, -2);
        return 2;
    }
    if (hasEnvironment) {
        lua_pushvalue(L, 3);
        lua_setupvalue(L, -2, 1);
    }
    return 1;
}

int execute(lua_State* L) {
    lua_settop(L, 1);
    // compile() returns before calling Lua: no C++ objects cross lua_call's longjmp.
    if (compile(L) != 1)
        return lua_error(L);
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - 1;
}

int search(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    auto* storage = static_cast<StorageSystem*>(lua_touserdata(L, lua_upvalueindex(1)));
    std::string module = name;
    for (auto& ch : module)
        if (ch == '.')
            ch = '/';
    for (const char* suffix : {".lua", "/init.lua"}) {
        const auto path = resolve(L, (module + suffix).c_str());
        std::string source;
        if (!path.empty() && storage->readTextFile(path.c_str(), source)) {
            if (source.compare(0, 3, "\xEF\xBB\xBF") == 0)
                source.erase(0, 3);
            if (luaL_loadbuffer(L, source.data(), source.size(), path.c_str()) != LUA_OK)
                return 1;
            lua_pushstring(L, path.c_str());
            return 2;
        }
    }
    lua_pushfstring(L, "\n\tno game module '%s'", name);
    return 1;
}

void pushLoader(lua_State* L, StorageSystem* storage, const std::string& root,
                lua_CFunction function) {
    lua_pushlightuserdata(L, storage);
    lua_pushlstring(L, root.data(), root.size());
    lua_pushcclosure(L, function, 2);
}
}  // namespace

void installGameAssetLoaders(lua_State* L, StorageSystem* storage, const std::string& entryPoint) {
    const auto path = GameAssetCache::normalize(entryPoint.c_str());
    if (path.compare(0, 7, "/games/") != 0)
        return;
    const auto root = path.substr(0, path.find_last_of('/'));
    pushLoader(L, storage, root, compile);
    lua_setglobal(L, "loadfile");
    pushLoader(L, storage, root, execute);
    lua_setglobal(L, "dofile");
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "searchers");
    for (int i = int(lua_rawlen(L, -1)) + 1; i > 2; --i) {
        lua_rawgeti(L, -1, i - 1);
        lua_rawseti(L, -2, i);
    }
    pushLoader(L, storage, root, search);
    lua_rawseti(L, -2, 2);
    lua_pop(L, 2);
}
