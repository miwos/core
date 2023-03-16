#ifndef LuaBridgeLib_h
#define LuaBridgeLib_h

#include <Bridge.h>
#include <helpers/Lua.h>

namespace BridgeLib {
  using Bridge::Data;
  using Bridge::RequestId;

  namespace {
    int handleOscRef = -1;
  };

  void begin() {
    Bridge::addMethod("/e/*/*", [](Data &data) {
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
        Bridge::serial->printf(
          "return type `%s` not supported in request `%s`",
          lua_typename(Lua::L, -1),
          address
        );
        Bridge::endRespond();
      }
    });
  }

  namespace lib {
    int notify(lua_State *L) {
      const char *address = luaL_checkstring(L, 1);
      byte numArguments = lua_gettop(L);
      OSCMessage message(address);

      // Skip first argument (osc address).
      for (byte i = 2; i <= numArguments; i++) {
        int type = lua_type(L, i);
        if (type == LUA_TBOOLEAN) {
          message.add(lua_toboolean(L, i));
        } else if (type == LUA_TNUMBER) {
          message.add(lua_tonumber(L, i));
        } else if (type == LUA_TSTRING) {
          message.add(lua_tostring(L, i));
        }
      }

      Bridge::sendOscMessage(message);
      return 0;
    }

  } // namespace lib

  void install() {
    handleOscRef = -1;
    luaL_Reg lib[] = {{"notify", lib::notify}, {NULL, NULL}};
    luaL_register(Lua::L, "Bridge", lib);
  }

} // namespace BridgeLib

#endif