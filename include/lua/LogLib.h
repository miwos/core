#ifndef LuaLogLib_h
#define LuaLogLib_h

#include <Logger.h>
#include <helpers/Lua.h>

namespace LogLib {
  namespace lib {
    using Logger::beginLog;
    using Logger::endLog;
    using Logger::log;
    using Logger::LogType;
    using Logger::serial;

    int log(lua_State *L) {
      // Use zero-based index
      auto type = static_cast<LogType>(luaL_checkinteger(L, -2) - 1);

      const char *text = luaL_checkstring(L, -1);
      log(type, text);

      return 0;
    }

    int flush(lua_State *L) {
      serial->endPacket();
      return 0;
    }

    int stack(lua_State *L) {
      int top = lua_gettop(L);
      if (top == 0) {
        log("/log/stack", "{}");
        return 0;
      }

      beginLog("/raw/log/stack");
      serial->print('{');

      for (int i = 1; i <= top; i++) {
        int type = lua_type(L, i);
        if (type == LUA_TNUMBER) {
          serial->printf(F("%g"), lua_tonumber(L, i));
        } else if (type == LUA_TSTRING) {
          serial->printf(F("'%s'"), lua_tostring(L, i));
        } else if (type == LUA_TBOOLEAN) {
          serial->printf(lua_toboolean(L, i) ? F("true") : F("false"));
        } else if (type == LUA_TNIL) {
          serial->print(F("'#nil#'"));
        } else {
          serial->printf(F("'#%s#'"), luaL_typename(L, i));
        }
        serial->print(',');
      }

      serial->print('}');
      endLog();

      return 0;
    }

  } // namespace lib

  void install() {
    luaL_Reg lib[] = {
      {"_log", lib::log},
      {"flush", lib::flush},
      {"stack", lib::stack},
      {NULL, NULL}};

    luaL_register(Lua::L, "Log", lib);
  }
} // namespace LogLib

#endif