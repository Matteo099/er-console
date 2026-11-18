#include "GameSceneBindings.h"

#include "scenes/game/GameScene.h"
#include "log/Logger.h"
#include <cmath>

GameSceneBindings::GameSceneBindings(GameScene* scene) : scene(scene) {
}

GameScene* GameSceneBindings::get(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, GAME_SCENE_PTR_KEY);
    auto* scene = static_cast<GameScene*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return scene;
}

// @doc Complete the escape room successfully
// @example scene.completeGame()
// @lua scene.completeGame() -> void
int GameSceneBindings::completeGame(lua_State* L) {
    auto* scene = get(L);

    Log::info("GameSceneBindings", "Lua script requested game completion");

    if (!scene) {
        Log::warn("GameSceneBindings", "completeGame called but GameScene is null");
        return 0;
    }

    scene->requestCompletion();  // Consumed by GameScene on the UI task.

    return 0;
}

// @doc Set the interactive objects shown in the game object viewer.
// @param objects Array of game objects.
// Each object contains:
//   id: optional stable identifier (recommended for localized titles)
//   title: string
//   image: optional reveal image, relative to the game directory or absolute
//   content: array of blocks
//
// A content block can be:
//   {
//      type = GameObjectContentType.TEXT,
//      value = "some paragraph"
//   }
//
// or:
//
//   {
//      type = GameObjectContentType.IMAGE,
//      path = "/images/map.png",
//      width = 200,
//      height = 120
//   }
//
// Text and images can be mixed freely.
//
// @example
// scene.setGameObjects({
//     {
//         title = "Ancient Map",
//         content = {
//             {
//                 type = GameObjectContentType.TEXT,
//                 value = "A mysterious map is found on the wall."
//             },
//             {
//                 type = GameObjectContentType.IMAGE,
//                 path = "/games/demo/map.png",
//                 width = 220,
//                 height = 160
//             },
//             {
//                 type = GameObjectContentType.TEXT,
//                 value = "The red mark indicates the hidden door."
//             }
//         }
//     }
// })
//
// @lua scene.setGameObjects(objects:GameObject[]) -> void
namespace {
std::string objectPath(GameScene* scene, const char* value) {
    std::string path = value ? value : "";
    if (!path.empty() && path[0] != '/')
        path = scene->getGameDataDirectory() + "/" + path;
    return path;
}

GameObjectSubview::ObjectData readObject(lua_State* L, int index, GameScene* scene) {
    index = lua_absindex(L, index);
    GameObjectSubview::ObjectData object;
    lua_getfield(L, index, "title");
    if (lua_isstring(L, -1))
        object.title = lua_tostring(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, index, "id");
    if (lua_isstring(L, -1))
        object.id = lua_tostring(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, index, "image");
    if (lua_isstring(L, -1))
        object.image = objectPath(scene, lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, index, "content");
    if (lua_istable(L, -1)) {
        const auto count = lua_rawlen(L, -1);
        for (size_t i = 1; i <= count; ++i) {
            lua_rawgeti(L, -1, i);
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "type");
                const char* type = lua_tostring(L, -1);
                lua_pop(L, 1);
                if (type && !strcmp(type, "text")) {
                    lua_getfield(L, -1, "value");
                    if (lua_isstring(L, -1))
                        object.content.push_back(
                            GameObjectSubview::ContentBlock::textBlock(lua_tostring(L, -1)));
                    lua_pop(L, 1);
                } else if (type && !strcmp(type, "image")) {
                    lua_getfield(L, -1, "path");
                    const auto path = lua_isstring(L, -1) ? objectPath(scene, lua_tostring(L, -1))
                                                          : std::string();
                    lua_pop(L, 1);
                    int width = LV_SIZE_CONTENT, height = LV_SIZE_CONTENT;
                    lua_getfield(L, -1, "width");
                    if (lua_isinteger(L, -1))
                        width = lua_tointeger(L, -1);
                    lua_pop(L, 1);
                    lua_getfield(L, -1, "height");
                    if (lua_isinteger(L, -1))
                        height = lua_tointeger(L, -1);
                    lua_pop(L, 1);
                    if (!path.empty())
                        object.content.push_back(
                            GameObjectSubview::ContentBlock::imageBlock(path, width, height));
                }
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    return object;
}
}  // namespace

namespace {
void requireHintId(lua_State* L, int index) {
    lua_getfield(L, index, "id");
    luaL_checktype(L, -1, LUA_TSTRING);
    if (!lua_rawlen(L, -1))
        luaL_error(L, "hint.id must not be empty");
    lua_pop(L, 1);
    lua_getfield(L, index, "title");
    luaL_checktype(L, -1, LUA_TSTRING);
    if (!lua_rawlen(L, -1))
        luaL_error(L, "hint.title must not be empty");
    lua_pop(L, 1);
}
}  // namespace

// @doc Configure timed hints for each active room step; time excludes pauses.
// @lua scene.setHints(hints:Hint[]) -> void
int GameSceneBindings::setHints(lua_State* L) {
    auto* scene = get(L);
    if (!scene)
        return 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    // Validate before creating C++ containers: Lua errors use longjmp.
    const auto count = lua_rawlen(L, 1);
    lua_newtable(L);
    const int seen = lua_absindex(L, -1);
    for (size_t i = 1; i <= count; ++i) {
        lua_rawgeti(L, 1, i);
        luaL_checktype(L, -1, LUA_TTABLE);
        const int entry = lua_absindex(L, -1);
        requireHintId(L, entry);
        lua_getfield(L, entry, "id");
        lua_pushvalue(L, -1);
        lua_rawget(L, seen);
        if (!lua_isnil(L, -1))
            return luaL_error(L, "duplicate hint.id");
        lua_pop(L, 1);
        lua_pushboolean(L, true);
        lua_rawset(L, seen);
        lua_getfield(L, entry, "step");
        luaL_checktype(L, -1, LUA_TSTRING);
        if (!lua_rawlen(L, -1))
            return luaL_error(L, "hint.step must not be empty");
        lua_pop(L, 1);
        lua_getfield(L, entry, "afterSeconds");
        luaL_checktype(L, -1, LUA_TNUMBER);
        const auto seconds = lua_tonumber(L, -1);
        if (!std::isfinite(seconds) || seconds < 0 || seconds > double(UINT32_MAX) / 1000.0)
            return luaL_error(L, "hint.afterSeconds must be finite and non-negative");
        lua_pop(L, 2);
    }
    lua_pop(L, 1);
    std::vector<HintDefinition> definitions;
    for (size_t i = 1; i <= count; ++i) {
        lua_rawgeti(L, 1, i);
        HintDefinition definition;
        definition.data = readObject(L, -1, scene);
        lua_getfield(L, -1, "step");
        definition.step = lua_tostring(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "afterSeconds");
        definition.afterMs = uint32_t(lua_tonumber(L, -1) * 1000.0 + 0.5);
        lua_pop(L, 2);
        definitions.push_back(std::move(definition));
    }
    scene->postHints(std::move(definitions));
    return 0;
}

// @doc Set the current objective/step. Repeating the same step keeps its timer.
// @lua scene.setStep(step:string) -> void
int GameSceneBindings::setStep(lua_State* L) {
    auto* scene = get(L);
    if (!scene)
        return 0;
    luaL_checktype(L, 1, LUA_TSTRING);
    scene->postGameStep(lua_tostring(L, 1));
    return 0;
}

// @doc Unlock a rich hint explicitly for custom Lua conditions; IDs are deduplicated.
// @lua scene.unlockHint(hint:GameObject) -> void
int GameSceneBindings::unlockHint(lua_State* L) {
    auto* scene = get(L);
    if (!scene)
        return 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    requireHintId(L, 1);
    scene->postHint(readObject(L, 1, scene));
    return 0;
}

int GameSceneBindings::setGameObjects(lua_State* L) {
    auto* scene = get(L);
    if (!scene)
        return 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    std::vector<GameObjectSubview::ObjectData> objects;
    for (size_t i = 1, count = lua_rawlen(L, 1); i <= count; ++i) {
        lua_rawgeti(L, 1, i);
        if (lua_istable(L, -1))
            objects.push_back(readObject(L, -1, scene));
        lua_pop(L, 1);
    }
    scene->postGameObjects(std::move(objects));
    return 0;
}

// @doc Discover or update one item. New IDs are revealed, then added to inventory.
// @lua scene.discoverGameObject(object:GameObject) -> void
int GameSceneBindings::discoverGameObject(lua_State* L) {
    auto* scene = get(L);
    if (!scene)
        return 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    scene->postGameObject(readObject(L, 1, scene));
    return 0;
}

void GameSceneBindings::registerAPI(lua_State* L) {
    lua_pushlightuserdata(L, scene);
    lua_setfield(L, LUA_REGISTRYINDEX, GAME_SCENE_PTR_KEY);

    lua_newtable(L);

    lua_pushcfunction(L, completeGame);
    lua_setfield(L, -2, "completeGame");

    lua_pushcfunction(L, setGameObjects);
    lua_setfield(L, -2, "setGameObjects");

    lua_pushcfunction(L, discoverGameObject);
    lua_setfield(L, -2, "discoverGameObject");

    lua_pushcfunction(L, setHints);
    lua_setfield(L, -2, "setHints");
    lua_pushcfunction(L, setStep);
    lua_setfield(L, -2, "setStep");
    lua_pushcfunction(L, unlockHint);
    lua_setfield(L, -2, "unlockHint");

    lua_setglobal(L, "scene");
}
