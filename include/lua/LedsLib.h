#ifndef LuaLedsLib_h
#define LuaLedsLib_h

#include <Arduino.h>
#include <Logger.h>

namespace LedsLib {
  using Logger::beginError;
  using Logger::endError;
  using Logger::serial;

  struct Led {
    byte pin;
    bool pwm;
  };

  const byte maxLeds = 7;
  Led leds[maxLeds] = {
    {9, true},
    {11, true},
    {27, false},
    {14, true},
    {30, false},
    {29, true},
    {0, true},
  };

  byte pwmResolution = 8;
  float pwmFrequency = 585937.5;
  byte pins[maxLeds] = {9, 11, 27, 14, 30, 29, 0};

  void write(byte index, byte intensity) {
    if (index >= maxLeds) {
      beginError();
      // Increase the index to be consistent with lua's index.
      serial->printf(F("LED #%d doesn't exist."), index + 1);
      endError();
      return;
    }
    byte pin = leds[index].pin;
    if (leds[index].pwm) {
      analogWrite(pin, intensity);
    } else {
      digitalWrite(pin, intensity == 0 ? LOW : HIGH);
    }
  }

  void begin() {
    analogWriteResolution(pwmResolution);
    for (byte i = 0; i < maxLeds; i++) {
      byte pin = leds[i].pin;
      pinMode(pin, OUTPUT);
      if (leds[i].pwm) analogWriteFrequency(pin, pwmFrequency);
      write(i, 0);
    }
  }

  namespace lib {
    int write(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte intensity = luaL_checknumber(L, 2);
      LedsLib::write(index, intensity);
      return 0;
    }

  } // namespace lib
  void install() {
    const luaL_reg library[] = {{"write", lib::write}, {NULL, NULL}};
    luaL_register(Lua::L, "Leds", library);
  }
}; // namespace LedsLib

#endif