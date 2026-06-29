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

void GameSceneBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);

    lua_pushcfunction(L, completeGame);
    lua_setfield(L, -2, "completeGame");

    lua_setglobal(L, "scene");
}