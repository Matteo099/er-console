#include "GameSceneBindings.h"

#include "scenes/game/GameScene.h"
#include "log/Logger.h"

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

    lua_setglobal(L, "scene");
}
