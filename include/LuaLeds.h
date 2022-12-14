#ifndef LuaLeds_h
#define LuaLeds_h

#include <Leds.h>
#include <Lua.h>

namespace LuaLeds {
  int write(lua_State *L) {
    byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
    byte intensity = luaL_checknumber(L, 2);
    Leds::write(index, intensity);
    return 0;
  }

  void install() {
    const luaL_reg library[] = {{"write", write}, {NULL, NULL}};
    luaL_register(Lua::L, "Leds", library);
  }
} // namespace LuaLeds

#endif