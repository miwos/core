#ifndef LuaUtilsLib_h
#define LuaUtilsLib_h

#include <helpers/Lua.h>

namespace UtilsLib {
  namespace lib {
    int packBytes(lua_State *L) {
      byte numArguments = lua_gettop(L);

      uint32_t packed = 0;
      for (byte i = 0; i < numArguments; i++) {
        byte number = luaL_checkinteger(L, i + 1);
        packed |= (number & 0xFF) << (i * 8);
      }

      lua_pushnumber(L, packed);
      return 1;
    }

    int unpackBytes(lua_State *L) {
      uint32_t packed = luaL_checkinteger(L, 1);
      lua_pushnumber(L, packed & 0xFF);
      lua_pushnumber(L, (packed >> 8) & 0xFF);
      lua_pushnumber(L, (packed >> 16) & 0xFF);
      lua_pushnumber(L, (packed >> 24) & 0xFF);
      return 4;
    }

    int setBit(lua_State *L) {
      uint32_t number = luaL_checkinteger(L, 1);
      byte position = luaL_checkinteger(L, 2) - 1; // use zero-based index
      byte value = lua_toboolean(L, 3);
      if (value) {
        bitSet(number, position);
      } else {
        bitClear(number, position);
      }
      lua_pushnumber(L, number);
      return 1;
    }

  } // namespace lib

  void install() {
    luaL_Reg lib[] = {
      {"packBytes", lib::packBytes},
      {"unpackBytes", lib::unpackBytes},
      {"setBit", lib::setBit},
      {NULL, NULL}};

    luaL_register(Lua::L, "Utils", lib);
  }
} // namespace UtilsLib

#endif