#ifndef LuaTimer_h
#define LuaTimer_h

#include <Arduino.h>
#include <Lua.h>

namespace LuaTimer {
  uint32_t currentTime = 0;

  namespace {
    int updateRef = -1;
  } // namespace

  int now(lua_State *L) {
    lua_pushinteger(Lua::L, micros());
    return 1;
  }

  void install() {
    updateRef = -1;
    luaL_Reg lib[] = {{"now", now}, {NULL, NULL}};
    luaL_register(Lua::L, "Timer", lib);
  }

  void update() {
    currentTime = micros();

    // Don't log an error if we can't find the function because this gets called
    // thousands of times per second!
    if (updateRef == -1)
      updateRef = Lua::storeFunction("Timer", "update", false);

    if (Lua::getFunction(updateRef, false)) {
      lua_pushinteger(Lua::L, currentTime);
      // ? Maybe add a debug flag and use lua_call instead of lua_pcall in
      // ? production because it is faster.
      Lua::check(lua_pcall(Lua::L, 1, 0, 0));
    }
  }
} // namespace LuaTimer

#endif