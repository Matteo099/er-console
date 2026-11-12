#pragma once

#include "LuaBinding.h"

class GameScene;

class GameSceneBindings : public LuaBinding {
  public:
    explicit GameSceneBindings(GameScene* scene);
    void registerAPI(lua_State* L) override;

  private:
    GameScene* scene;

  private:
    static GameScene* get(lua_State* L);

    static int completeGame(lua_State* L);
    static int setGameObjects(lua_State* L);
};