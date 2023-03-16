#ifndef LuaMidi_h
#define LuaMidi_h

#include <IntervalTimer.h>
#include <Lua.h>
#include <LuaTimer.h>
#include <MidiDevices.h>

namespace LuaMidi {
  int handleInputRef = -1;
  int handleClockRef = -1;
  IntervalTimer clockTimer;
  float bpm = 120.0;
  int ppq = 24;
  uint32_t currentTick = 0;

  int send(lua_State *L) {
    byte index = lua_tonumber(L, 1) - 1; // Use zero-based index.
    byte type = lua_tonumber(L, 2);
    byte data1 = lua_tonumber(L, 3);
    byte data2 = lua_tonumber(L, 4);
    byte channel = lua_tonumber(L, 5);   // Channel is always a one-based index.
    byte cable = lua_tonumber(L, 6) - 1; // Use zero-based index.

    AnyMidi *device = MidiDevices::getDevice(index);
    if (device != NULL) device->send(type, data1, data2, channel, cable);
    return 0;
  }

  void handleInput(byte index, byte type, byte data1, byte data2, byte channel,
      byte cable = 0) {

    if (handleInputRef == -1)
      handleInputRef = Lua::storeFunction("Midi", "handleInput");

    if (!Lua::getFunction(handleInputRef)) return;

    lua_pushnumber(Lua::L, index + 1); // Use one-based index.
    lua_pushnumber(Lua::L, type);
    lua_pushnumber(Lua::L, data1);
    lua_pushnumber(Lua::L, data2);
    lua_pushnumber(Lua::L, channel);   // Channel is already a one-based index.
    lua_pushnumber(Lua::L, cable + 1); // Use one-based index.
    Lua::check(lua_pcall(Lua::L, 6, 0, 0));
  }

  void handleTimerEvent(uint32_t data) {
    uint32_t message = data >> 8;
    byte status = message >> 16;
    byte type = status & 0xF0;
    byte channel = (status & 15) + 1; // Channel is stored zero-based.
    byte data1 = (message >> 8) & 127;
    byte data2 = message & 127;
    byte deviceIndex = (data >> 4) & 15;
    byte cable = data & 15;

    AnyMidi *device = MidiDevices::getDevice(deviceIndex);
    if (device != NULL) device->send(type, data1, data2, channel, cable);
  }

  void handleMidiClock() {
    // TODO: sync selected midi devices
    usbMIDI.sendRealTime(usbMIDI.Clock);
    LuaTimer::updateEvents(currentTick, true);
    currentTick++;
    LuaTimer::currentTick = currentTick;
  }

  int start(lua_State *L) {
    // TODO: sync selected midi devices
    usbMIDI.sendRealTime(usbMIDI.Start);
    clockTimer.begin(handleMidiClock, (60000000 / bpm) / ppq);
    return 0;
  }

  int setTempo(lua_State *L) {
    bpm = luaL_checknumber(L, 1);
    clockTimer.update((60000000 / bpm) / ppq);
    return 0;
  }

  int stop(lua_State *L) {
    // TODO: sync selected midi devices
    usbMIDI.sendRealTime(usbMIDI.Stop);
    clockTimer.end();
    return 0;
  }

  void begin() {
    for (byte i = 0; i < MidiDevices::maxDevices; i++) {
      AnyMidi *device = MidiDevices::getDevice(i);
      device->onInput(handleInput);
    }
    LuaTimer::onEvent(handleTimerEvent);
  }

  void install() {
    handleInputRef = -1;
    handleClockRef = -1;

    luaL_Reg lib[] = {{"__send", send}, {"start", start}, {"stop", stop},
        {"setTempo", setTempo}, {NULL, NULL}};
    luaL_register(Lua::L, "Midi", lib);
  }
} // namespace LuaMidi

#endif