#include "TimeBindings.h"

#include <Arduino.h>

float TimeBindings::s_delta = 0.0f;

void TimeBindings::setDelta(float dt) { s_delta = dt; }

/**
 * time.now() -- milliseconds since boot
 */
int TimeBindings::now(lua_State *L)
{
    lua_pushinteger(L, millis());
    return 1;
}

/**
 * time.seconds() -- seconds since boot
 */
int TimeBindings::seconds(lua_State *L)
{
    lua_pushnumber(L, millis() / 1000.0);
    return 1;
}

/**
 * time.uptime() -- alias of time.seconds()
 */
int TimeBindings::uptime(lua_State *L)
{
    lua_pushnumber(L, millis() / 1000.0);
    return 1;
}

/**
 * time.clock() -- "HH:MM:SS"
 */
int TimeBindings::clock(lua_State *L)
{
    unsigned long ms = millis();
    int seconds = (ms / 1000) % 60;
    int minutes = (ms / 60000) % 60;
    int hours = (ms / 3600000) % 24;
    char buffer[16];
    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
    lua_pushstring(L, buffer);
    return 1;
}

/**
 * time.delta() -- frame delta
 */
int TimeBindings::delta(lua_State *L)
{
    lua_pushnumber(L, s_delta);
    return 1;
}

TimeBindings::TimeBindings() {}

void TimeBindings::registerAPI(lua_State *L)
{
    lua_newtable(L);
    lua_pushcfunction(L, now);
    lua_setfield(L, -2, "now");
    lua_pushcfunction(L, seconds);
    lua_setfield(L, -2, "seconds");
    lua_pushcfunction(L, uptime);
    lua_setfield(L, -2, "uptime");
    lua_pushcfunction(L, clock);
    lua_setfield(L, -2, "clock");
    lua_pushcfunction(L, delta);
    lua_setfield(L, -2, "delta");
    lua_setglobal(L, "time");
}