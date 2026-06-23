#pragma once

#include "LuaBinding.h"

class GameScene;
class GameSceneBindings : public LuaBinding
{
public:
    explicit GameSceneBindings(GameScene *scene);
    void registerAPI(lua_State *L) override;

private:
    GameScene *scene;

private:
    static GameScene *get(lua_State *L);

    static int completeGame(lua_State *L);

    static int clearDisplay(lua_State *L);
    static int drawText(lua_State *L);
    static int drawRect(lua_State *L);
    static int drawLine(lua_State *L);

    static int createButton(lua_State *L);
    static int setButtonCallback(lua_State *L);
    static int removeButton(lua_State *L);
};