#ifndef LuaButtons_h
#define LuaButtons_h

#include <Button.h>
#include <Lua.h>

namespace LuaButtons {
  int read(lua_State *L) {
    byte index = luaL_checknumber(Lua::L, 1) - 1; // zero-based index
    Button *button = Buttons::getButton(index);
    if (button == NULL) return 1;

    lua_pushboolean(Lua::L, button->read());
    return 0;
  }

  void handleClick(byte encoderIndex, uint32_t duration) {
    if (!Lua::getFunction("Buttons", "handleClick")) return;
    lua_pushinteger(Lua::L, encoderIndex + 1); // one-based index
    lua_pushinteger(Lua::L, duration);
    lua_call(Lua::L, 2, 0);
  }

  void begin() { Buttons::onClick(handleClick); }

  void install() {
    luaL_Reg lib[] = {{"read", read}, {NULL, NULL}};
    luaL_register(Lua::L, "Buttons", lib);
  }
} // namespace LuaButtons

#endif