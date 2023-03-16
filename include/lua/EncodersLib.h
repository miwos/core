#ifndef LuaEncodersLib_h
#define LuaEncodersLib_h

#include <Logger.h>
#include <RangeEncoder.h>
#include <helpers/Lua.h>

namespace EncodersLib {
  using Logger::beginError;
  using Logger::endError;
  using Logger::serial;

  typedef RangeEncoder Encoder;
  const byte maxEncoders = 3;
  RangeEncoder encoders[maxEncoders] = {
    RangeEncoder(3, 2, 0, 127),
    RangeEncoder(36, 37, 0, 127),
    RangeEncoder(33, 34, 0, 127)};

  uint32_t lastUpdate = 0;
  uint32_t updateInterval = 5; // ms

  RangeEncoder *getEncoder(byte index) {
    if (index >= maxEncoders) {
      beginError();
      // Use one-based index.
      serial->printf(F("Encoder #%d doesn't exist."), index + 1);
      endError();
      return NULL;
    }
    return &(encoders[index]);
  }

  void handleChange(byte encoderIndex, int32_t value) {
    if (!Lua::getFunction("Encoders", "handleChange")) return;
    lua_pushinteger(Lua::L, encoderIndex + 1); // one-based index
    lua_pushinteger(Lua::L, value);
    Lua::check(lua_pcall(Lua::L, 2, 0, 0));
  }

  void update() {
    uint32_t now = millis();
    if (now - lastUpdate < updateInterval) return;

    for (byte i = 0; i < maxEncoders; i++) {
      bool changed;
      int32_t value = encoders[i].read(changed);
      if (changed) handleChange(i, value);
    }

    lastUpdate = now;
  }

  namespace lib {
    int write(lua_State *L) {
      byte index = lua_tonumber(Lua::L, 1) - 1; // zero-based index
      int32_t value = lua_tonumber(Lua::L, 2);
      RangeEncoder *encoder = getEncoder(index);
      if (encoder != NULL) encoder->write(value);
      return 0;
    }

    int setRange(lua_State *L) {
      byte index = luaL_checknumber(Lua::L, 1) - 1; // zero-based index.
      int32_t min = luaL_checkinteger(Lua::L, 2);
      int32_t max = luaL_checkinteger(Lua::L, 3);

      RangeEncoder *encoder = getEncoder(index);
      if (encoder != NULL) encoder->setRange(min, max);
      return 0;
    }
  } // namespace lib

  void install() {
    luaL_Reg lib[] = {
      {"write", lib::write}, {"setRange", lib::setRange}, {NULL, NULL}};
    luaL_register(Lua::L, "Encoders", lib);
  }
} // namespace EncodersLib

#endif