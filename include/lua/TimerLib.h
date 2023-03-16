#ifndef LuaTimerLib_h
#define LuaTimerLib_h

#include <Arduino.h>
#include <Logger.h>
#include <helpers/Lua.h>

namespace TimerLib {
  using Logger::beginInfo;
  using Logger::endInfo;
  using Logger::serial;
  using Logger::warn;

  typedef void (*EventHandler)(uint32_t data);
  EventHandler handleEvent;
  uint32_t currentTick = 0; // updated by `Midi`.

  struct MidiEvent {
    uint32_t time;
    uint32_t data;
  };

  const byte MAX_EVENTS = 255;
  MidiEvent events[MAX_EVENTS];

  uint32_t currentTime = 0;

  namespace {
    int updateRef = -1;
  } // namespace

  void updateEvents(uint32_t time, bool useTicks) {
    for (byte i = 0; i < MAX_EVENTS; i++) {
      bool eventUsesTicks = events[i].data & (1 << 31);
      // Restore the MSB, see `LuaTimer::scheduleMidi()`.
      uint32_t data = events[i].data | (1 << 31);

      if (useTicks && !eventUsesTicks) continue;
      if (events[i].time > time) continue;

      if (handleEvent != NULL) handleEvent(data);
      // A data of zero acts as a flag for a free event.
      events[i].data = 0;
    }
  }

  void update() {
    currentTime = ::micros();

    // Don't log an error if we can't find the function because this gets called
    // thousands of times per second!
    if (updateRef == -1)
      updateRef = Lua::storeFunction("Timer", "update", false);

    if (Lua::getFunction(updateRef, false)) {
      lua_pushinteger(Lua::L, currentTime);
      // ? Maybe add a debug flag and use lua_call instead of lua_pcall in
      // ? production because it is faster.
      Lua::check(lua_pcall(Lua::L, 1, 0, 0));
    }
  }

  namespace lib {

    int millis(lua_State *L) {
      lua_pushinteger(Lua::L, ::millis());
      return 1;
    }

    int micros(lua_State *L) {
      lua_pushinteger(Lua::L, ::micros());
      return 1;
    }

    int ticks(lua_State *L) {
      lua_pushinteger(Lua::L, currentTick);
      return 1;
    }

    int scheduleMidi(lua_State *L) {
      int availableEventIndex = -1;
      for (byte i = 0; i < MAX_EVENTS; i++) {
        if (events[i].data == 0) {
          availableEventIndex = i;
          break;
        }
      }

      if (availableEventIndex == -1) {
        warn("all midi-events in use");
        lua_pushboolean(L, false);
        return 1;
      }

      uint32_t time = luaL_checknumber(L, 1);
      bool useTicks = lua_toboolean(L, 2);
      byte type = luaL_checkint(L, 3);
      byte data1 = luaL_checkint(L, 4);
      byte data2 = luaL_checkint(L, 5);
      byte channel =
        luaL_checkint(L, 6) - 1; // use zero-based index (for storage)
      byte deviceIndex = luaL_checkint(L, 7) - 1; // use zero-based index
      byte cable = luaL_checkint(L, 8) - 1; // use zero-based index

      byte status = type | channel;
      uint32_t message = (status << 16) | (data1 << 8) | data2;
      uint32_t data = (message << 8) | (deviceIndex << 4) | cable;

      // To save some space, we store wether to use ticks or ms as a flag in the
      // most significant bit (32). This bit is already used by the midi message
      // but only to indicate a status byte (which for the first byte in a midi
      // message is always the case). We can therefore use it, as long as we
      // make sure to set it back to 1 before we send the message.
      if (useTicks) {
        // MSB is already set to 1.
      } else {
        // Set MSB to 0.
        data = data | ~(1 << 31);
      }

      MidiEvent midiEvent;
      midiEvent.time = time;
      midiEvent.data = data;
      events[availableEventIndex] = midiEvent;

      lua_pushboolean(L, true);
      return 1;
    }
  } // namespace lib

  void install() {
    updateRef = -1;

    luaL_Reg lib[] = {
      {"millis", lib::millis},
      {"micros", lib::micros},
      {"ticks", lib::ticks},
      {"_scheduleMidi", lib::scheduleMidi},
      {NULL, NULL}};

    luaL_register(Lua::L, "Timer", lib);
  }

  void onEvent(EventHandler handler) {
    handleEvent = handler;
  }
} // namespace TimerLib

#endif