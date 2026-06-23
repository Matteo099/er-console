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

// @doc Clears the game drawing area
// @lua scene.clearDisplay() -> void
int GameSceneBindings::clearDisplay(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    scene->getGameView()->clearDisplay();

    return 0;
}

// @doc Draw text on screen
// @param x X position
// @param y Y position
// @param text Text content
// @param fontSize Font size
// @param color RGB color
// @lua scene.drawText(x:int,y:int,text:string,fontSize:int,color:uint32) -> void
int GameSceneBindings::drawText(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);

    const char *text =
        luaL_checkstring(L, 3);

    int font =
        luaL_optinteger(L, 4, 16);

    uint32_t color =
        luaL_optinteger(L, 5, 0xFFFFFF);

    scene->getGameView()->drawText(
        x,
        y,
        text,
        font,
        color);

    return 0;
}

// @doc Draw rectangle on screen
// @param x X position
// @param y Y position
// @param width Width of rectangle
// @param height Height of rectangle
// @param color RGB color
// @param fill Whether to fill the rectangle
// @example scene.drawRect(10, 10, 100, 50, 0xFF0000, true)
// @lua scene.drawRect(x:int,y:int,width:int,height:int,color:uint32,fill:bool) -> void
int GameSceneBindings::drawRect(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);

    uint32_t color =
        luaL_optinteger(L, 5, 0xFFFFFF);

    bool fill =
        lua_toboolean(L, 6);

    scene->getGameView()->drawRect(
        x, y, w, h, color, fill);

    return 0;
}

// @doc Draw line on screen
// @param x1 X position of start point
// @param y1 Y position of start point
// @param x2 X position of end point
// @param y2 Y position of end point
// @param color RGB color
// @example scene.drawLine(10, 10, 100, 100, 0x00FF00)
// @lua scene.drawLine(x1:int,y1:int,x2:int,y2:int,color:uint32) -> void
int GameSceneBindings::drawLine(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);

    uint32_t color =
        luaL_optinteger(L, 5, 0xFFFFFF);

    scene->getGameView()->drawLine(
        x1, y1, x2, y2, color);

    return 0;
}

// @doc Create a button on the game view
// @param x X position
// @param y Y position
// @param width Width of the button
// @param height Height of the button
// @param text Text content of the button
// @example scene.createButton(10, 10, 100, 50, "Click Me")
// @lua button = scene.createButton(x,y,w,h,text)
int GameSceneBindings::createButton(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
    {
        lua_pushnil(L);
        return 1;
    }

    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int w = luaL_checkinteger(L, 3);
    int h = luaL_checkinteger(L, 4);

    const char *text =
        luaL_checkstring(L, 5);

    lv_obj_t *btn =
        scene->getGameView()
            ->createButton(
                x, y, w, h, text);

    lua_pushlightuserdata(
        L,
        btn);

    return 1;
}

// @doc Set a Lua callback for a button
// @param btn Button object
// @param callbackName Name of the Lua function to call when button is clicked
// @example scene.setButtonCallback(button, "onButtonClick")
// @lua scene.setButtonCallback(button:lv_obj_t, callbackName:string) -> void
int GameSceneBindings::setButtonCallback(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    lv_obj_t *btn =
        static_cast<lv_obj_t *>(
            lua_touserdata(L, 1));

    const char *callback =
        luaL_checkstring(L, 2);

    scene->getGameView()
        ->setButtonCallback(
            btn,
            callback);

    return 0;
}

// @doc Remove a button from the game view
// @param btn Button object
// @example scene.removeButton(button)
// @lua scene.removeButton(button:lv_obj_t) -> void
int GameSceneBindings::removeButton(lua_State *L)
{
    auto *scene = get(L);

    if (!scene)
        return 0;

    lv_obj_t *btn =
        static_cast<lv_obj_t *>(
            lua_touserdata(L, 1));

    scene->getGameView()
        ->removeButton(btn);

    return 0;
}

void GameSceneBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);

    lua_pushcfunction(L, completeGame);
    lua_setfield(L, -2, "completeGame");

    lua_pushcfunction(L, clearDisplay);
    lua_setfield(L, -2, "clearDisplay");

    lua_pushcfunction(L, drawText);
    lua_setfield(L, -2, "drawText");

    lua_pushcfunction(L, drawRect);
    lua_setfield(L, -2, "drawRect");

    lua_pushcfunction(L, drawLine);
    lua_setfield(L, -2, "drawLine");

    lua_pushcfunction(L, createButton);
    lua_setfield(L, -2, "createButton");

    lua_pushcfunction(L, setButtonCallback);
    lua_setfield(L, -2, "setButtonCallback");

    lua_pushcfunction(L, removeButton);
    lua_setfield(L, -2, "removeButton");

    lua_setglobal(L, "scene");
}