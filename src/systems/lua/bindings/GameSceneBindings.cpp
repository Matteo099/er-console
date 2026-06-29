#include "GameSceneBindings.h"

#include "scenes/game/GameScene.h"

GameSceneBindings::GameSceneBindings(GameScene *scene) : scene(scene) {}

GameScene *GameSceneBindings::get(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, GAME_SCENE_PTR_KEY);
    auto *scene = static_cast<GameScene *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return scene;
}

// @doc Complete the escape room successfully
// @example scene.completeGame()
// @lua scene.completeGame() -> void
int GameSceneBindings::completeGame(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    scene->completeGame();

    return 0;
}

// @doc Set the interactive objects shown in the game object viewer.
// @param objects Array of game objects.
// Each object contains:
//   title: string
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
int GameSceneBindings::setGameObjects(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    if (!lua_istable(L, 1))
        return luaL_error(L, "scene.setGameObjects expects a table");

    std::vector<GameObjectSubview::ObjectData> objects;
    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        /* value at -1 = object table */
        if (lua_istable(L, -1))
        {
            GameObjectSubview::ObjectData object;

            /* title */
            lua_getfield(L, -1, "title");
            if (lua_isstring(L, -1))
            {
                object.title = lua_tostring(L, -1);
            }
            lua_pop(L, 1);

            /* content */
            lua_getfield(L, -1, "content");
            if (lua_istable(L, -1))
            {
                lua_pushnil(L);
                while (lua_next(L, -2) != 0)
                {
                    if (lua_istable(L, -1))
                    {
                        lua_getfield(L, -1, "type");
                        const char *type = lua_tostring(L, -1);
                        lua_pop(L, 1);
                        if (type && strcmp(type, "text") == 0)
                        {
                            lua_getfield(L, -1, "value");
                            if (lua_isstring(L, -1))
                            {
                                object.content.push_back(GameObjectSubview::ContentBlock::textBlock(lua_tostring(L, -1)));
                            }
                            lua_pop(L, 1);
                        }
                        else if (type && strcmp(type, "image") == 0)
                        {
                            lua_getfield(L, -1, "path");
                            if (lua_isstring(L, -1))
                            {
                                std::string path = lua_tostring(L, -1);
                                if (path.empty() || path[0] != '/')
                                {
                                    // Relative path, prepend game data directory
                                    path = scene->getGameDataDirectory() + "/" + path;
                                }

                                int width = LV_SIZE_CONTENT;
                                int height = LV_SIZE_CONTENT;
                                lua_getfield(L, -1, "width");
                                if (lua_isinteger(L, -1))
                                    width = lua_tointeger(L, -1);
                                lua_pop(L, 1);
                                lua_getfield(L, -1, "height");
                                if (lua_isinteger(L, -1))
                                    height = lua_tointeger(L, -1);
                                lua_pop(L, 1);
                                object.content.push_back(GameObjectSubview::ContentBlock::imageBlock(path, width, height));
                            }
                            lua_pop(L, 1);
                        }
                    }
                    lua_pop(L, 1);
                }
            }
            lua_pop(L, 1);
            objects.push_back(object);
        }
        lua_pop(L, 1);
    }

    scene->getGameView()->setGameObjectData(objects);

    return 0;
}

void GameSceneBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);

    lua_pushcfunction(L, completeGame);
    lua_setfield(L, -2, "completeGame");

    lua_pushcfunction(L, setGameObjects);
    lua_setfield(L, -2, "setGameObjects");

    lua_setglobal(L, "scene");
}