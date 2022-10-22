#ifndef LuaBridge_h
#define LuaBridge_h

#include <Bridge.h>
#include <Lua.h>

namespace LuaBridge {
  using Bridge::Data;
  using Bridge::RequestId;

  namespace {
    int handleOscRef = -1;
  };

  void begin() {
    Bridge::addMethod("/*/*", [](Data &data) {
      RequestId id = data.getInt(0);
      byte numArguments = data.size();

      if (handleOscRef == -1)
        handleOscRef = Lua::storeFunction("Bridge", "handleOsc");
      if (!Lua::getFunction(handleOscRef)) return Bridge::respondError(id);

      static char address[256];
      data.getAddress(address, 0, 256);
      lua_pushstring(Lua::L, address);

      // Start with 1, because we want to ignore the first argument (id).
      for (int i = 1; i < numArguments; i++) {
        char type = data.getType(i);
        if (type == 'i') {
          lua_pushinteger(Lua::L, data.getInt(i));
        } else if (type == 'f') {
          lua_pushnumber(Lua::L, data.getFloat(i));
        } else if (type == 's') {
          char string[256];
          data.getString(i, string, 256);
          lua_pushstring(Lua::L, string);
        } else {
          Bridge::beginRespondError(id);
          Bridge::serial->printf(F("OSC type `%c` is not supported."), type);
          Bridge::endRespond();
          return;
        }
      }

      // Number of arguments is numArguments without the id + the address.
      if (lua_pcall(Lua::L, numArguments, 1, 0))
        return Bridge::respondError(id, lua_tostring(Lua::L, -1));

      auto returnType = lua_type(Lua::L, -1);
      if (returnType == LUA_TBOOLEAN) {
        return Bridge::respond(id, lua_toboolean(Lua::L, -1));
      } else if (returnType == LUA_TNUMBER) {
        return Bridge::respond(id, lua_tonumber(Lua::L, -1));
      } else if (lua_isstring(Lua::L, -1)) {
        return Bridge::respond(id, lua_tostring(Lua::L, -1));
      } else if (lua_isnil(Lua::L, -1)) {
        return Bridge::respond(id);
      } else {
        Bridge::beginRespondError(id);
        Bridge::serial->printf("return type `%s` not supported in request `%s`",
            lua_typename(Lua::L, -1), address);
        Bridge::endRespond();
      }
    });
  }

  void install() { handleOscRef = -1; }
} // namespace LuaBridge

#endif