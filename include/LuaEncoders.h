#ifndef LuaEncoders_h
#define LuaEncoders_h

#include <Encoders.h>
#include <Lua.h>

namespace LuaEncoders {
  int write(lua_State *L) {
    byte index = lua_tonumber(Lua::L, 1) - 1; // zero-based index
    int32_t value = lua_tonumber(Lua::L, 2);
    RangeEncoder *encoder = Encoders::getEncoder(index);
    if (encoder != NULL) encoder->write(value);
    return 0;
  }

  int setRange(lua_State *L) {
    byte index = luaL_checknumber(Lua::L, 1) - 1; // zero-based index.
    int32_t min = luaL_checkinteger(Lua::L, 2);
    int32_t max = luaL_checkinteger(Lua::L, 3);

    RangeEncoder *encoder = Encoders::getEncoder(index);
    if (encoder != NULL) encoder->setRange(min, max);
    return 0;
  }

  void handleChange(byte encoderIndex, int32_t value) {
    if (!Lua::getFunction("Encoders", "handleChange")) return;
    lua_pushinteger(Lua::L, encoderIndex + 1); // one-based index
    lua_pushinteger(Lua::L, value);
    lua_call(Lua::L, 2, 0);
  }

  void begin() { Encoders::onChange(handleChange); }

  void install() {
    luaL_Reg lib[] = {{"write", write}, {"setRange", setRange}, {NULL, NULL}};
    luaL_register(Lua::L, "Encoders", lib);
  }
} // namespace LuaEncoders

#endif