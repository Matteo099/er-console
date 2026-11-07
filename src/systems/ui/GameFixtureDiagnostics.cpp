#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "GameFixtureDiagnostics.h"
#include <Arduino.h>
#include "systems/lua/bindings/CoreBindings.h"
#include "localization/I18n.h"

void checkGameFixture(const std::string &source, bool english, bool objects)
{
    const auto previous = I18n::language();
    I18n::select(english ? I18n::Language::English : I18n::Language::Italian);
    auto *vm = luaL_newstate();
    if (!vm) { Serial.println("@GAMECHECK FAIL allocation"); I18n::select(previous); return; }
    luaL_openlibs(vm);
    CoreBindings().registerAPI(vm);
    unsigned count = 0;
    const auto chunk = source + "\nreturn messages\n";
    bool ok = luaL_loadbuffer(vm, chunk.data(), chunk.size(), "fixture") == LUA_OK && lua_pcall(vm,0,1,0) == LUA_OK && lua_istable(vm,-1);
    if (ok)
    {
        lua_pushnil(vm);
        while (lua_next(vm, 1))
        {
            const int message = lua_absindex(vm, -1);
            lua_getfield(vm, message, english ? "en" : "it");
            const char *expected = lua_tostring(vm,-1);
            const std::string text = expected ? expected : "";
            lua_pop(vm,1);
            lua_getglobal(vm,"system"); lua_getfield(vm,-1,"translate"); lua_remove(vm,-2);
            lua_pushvalue(vm,message);
            ok = !text.empty() && lua_pcall(vm,1,1,0) == LUA_OK && lua_type(vm,-1) == LUA_TSTRING && text == lua_tostring(vm,-1);
            if (!ok) break;
            ++count; lua_pop(vm,2);
        }
    }
    if (ok && objects)
    {
        lua_settop(vm,1); lua_pushvalue(vm,1); lua_setglobal(vm,"fixtureMessages");
        const char *mocks = R"lua(
            input = {channels = {pinMode = function() end, digitalRead = function() return false end}}
            PinMode = {INPUT_PULLUP = 2}
            GameObjectContentType = {TEXT = 'text', IMAGE = 'image'}
            scene = {setGameObjects = function(value) fixtureObjects = value end}
            init()
            function checkObjects()
                assert(#fixtureObjects == 2)
                assert(fixtureObjects[1].title == system.translate(fixtureMessages.map))
                assert(fixtureObjects[2].title == system.translate(fixtureMessages.board))
                assert(fixtureObjects[1].content[1].value == system.translate(fixtureMessages.mapDescription))
                assert(fixtureObjects[1].content[3].value == system.translate(fixtureMessages.mapHint))
            end
            checkObjects()
        )lua";
        ok = luaL_dostring(vm,mocks) == LUA_OK;
        if (ok)
        {
            I18n::select(english ? I18n::Language::Italian : I18n::Language::English);
            ok = luaL_dostring(vm,"update(0,true); checkObjects()") == LUA_OK;
        }
    }
    Serial.printf("@GAMECHECK %s lua language=%s messages=%u objects=%u\n", ok ? "PASS" : "FAIL", english ? "en" : "it", count, objects);
    if (!ok && lua_isstring(vm,-1)) Serial.printf("@GAMECHECK ERROR %s\n", lua_tostring(vm,-1));
    lua_close(vm);
    I18n::select(previous);
}
#endif
