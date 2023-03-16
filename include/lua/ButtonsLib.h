#ifndef LuaButtonsLib_h
#define LuaButtonsLib_h

#include <Arduino.h>
#include <Button.h>
#include <Logger.h>
#include <helpers/Lua.h>

namespace ButtonsLib {
  using Logger::beginError;
  using Logger::endError;
  using Logger::serial;

  uint32_t throttleInterval = 5; // ms
  uint32_t lastUpdate = 0;

  const byte maxButtons = 10;
  Button buttons[maxButtons] = {
    // Blue buttons
    Button(10),
    Button(26),
    Button(22),
    // Green buttons
    Button(15),
    Button(31),
    Button(32),
    // Encoder buttons
    Button(23),
    Button(6),
    Button(35),
    // Shift/Menu button
    Button(1)};

  Button *getButton(byte index) {
    if (index >= maxButtons) {
      beginError();
      // Use one-based index.
      serial->printf(F("Button #%d doesn't exist."), index + 1);
      endError();
      return NULL;
    }
    return &(buttons[index]);
  }

  void handleClick(byte encoderIndex, uint32_t duration) {
    if (!Lua::getFunction("Buttons", "handleClick")) return;
    lua_pushinteger(Lua::L, encoderIndex + 1); // one-based index
    lua_pushinteger(Lua::L, duration);
    Lua::check(lua_pcall(Lua::L, 2, 0, 0));
  }

  void begin() {
    for (byte i = 0; i < maxButtons; i++) {
      buttons[i].begin();
    }
  }

  void update() {
    uint32_t currentTime = millis();
    if (currentTime - lastUpdate < throttleInterval) return;

    bool clicked;
    uint32_t duration = 0;
    for (byte i = 0; i < maxButtons; i++) {
      clicked = buttons[i].clicked(duration);
      if (clicked) handleClick(i, duration);
    }

    lastUpdate = currentTime;
  }

  namespace lib {
    int read(lua_State *L) {
      byte index = luaL_checknumber(Lua::L, 1) - 1; // zero-based index
      Button *button = getButton(index);
      if (button == NULL) return 1;

      lua_pushboolean(Lua::L, button->read());
      return 0;
    }
  } // namespace lib

  void install() {
    luaL_Reg lib[] = {{"read", lib::read}, {NULL, NULL}};
    luaL_register(Lua::L, "Buttons", lib);
  }
} // namespace ButtonsLib

#endif