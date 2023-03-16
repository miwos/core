#ifndef LuaMidiLib_h
#define LuaMidiLib_h

#include <AnyMidi.h>
#include <AnyMidiSerial.h>
#include <AnyMidiUsb.h>
#include <AnyMidiUsbHub.h>
#include <IntervalTimer.h>
#include <Logger.h>
#include <MIDI.h>
#include <SPI.h>
#include <USBHost_t36.h>
#include <helpers/Lua.h>
#include <lua/TimerLib.h>

// Midi via USB/Power port.
AnyMidiUsb midiDevice1(0);

// Hardware Midi
MIDI_CREATE_INSTANCE(HardwareSerial, Serial5, serialMidi1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, serialMidi2);
AnyMidiSerial midiDevice2(1, &serialMidi1);
AnyMidiSerial midiDevice3(2, &serialMidi2);

// Usb Host Midi
USBHost usbHost;
USBHub hub1(usbHost);
USBHub hub2(usbHost);
USBHub hub3(usbHost);
USBHub hub4(usbHost);
MIDIDevice hubMidi1(usbHost);
MIDIDevice hubMidi2(usbHost);
MIDIDevice hubMidi3(usbHost);
MIDIDevice hubMidi4(usbHost);
MIDIDevice hubMidi5(usbHost);
MIDIDevice hubMidi6(usbHost);
MIDIDevice hubMidi7(usbHost);
MIDIDevice hubMidi8(usbHost);
MIDIDevice hubMidi9(usbHost);
MIDIDevice hubMidi10(usbHost);
AnyMidiUsbHub midiDevice4(3, &hubMidi1);
AnyMidiUsbHub midiDevice5(4, &hubMidi2);
AnyMidiUsbHub midiDevice6(5, &hubMidi3);
AnyMidiUsbHub midiDevice7(6, &hubMidi4);
AnyMidiUsbHub midiDevice8(7, &hubMidi5);
AnyMidiUsbHub midiDevice9(8, &hubMidi6);
AnyMidiUsbHub midiDevice10(9, &hubMidi7);
AnyMidiUsbHub midiDevice11(10, &hubMidi8);
AnyMidiUsbHub midiDevice12(11, &hubMidi9);
AnyMidiUsbHub midiDevice13(12, &hubMidi10);

namespace MidiLib {
  using Logger::beginError;
  using Logger::endError;
  using Logger::serial;

  typedef AnyMidi Device;
  const byte maxDevices = 13;
  Device *devices[maxDevices] = {
    &midiDevice1,
    &midiDevice2,
    &midiDevice3,
    &midiDevice4,
    &midiDevice5,
    &midiDevice6,
    &midiDevice7,
    &midiDevice8,
    &midiDevice9,
    &midiDevice10,
    &midiDevice11,
    &midiDevice12,
    &midiDevice13};

  int handleInputRef = -1;
  int handleClockRef = -1;
  IntervalTimer clockTimer;
  float bpm = 120.0;
  int ppq = 24;
  uint32_t currentTick = 0;

  Device *getDevice(byte index) {
    if (index >= maxDevices) {
      beginError();
      // Use one-based index.
      serial->printf(F("Midi device #%d not found"), index + 1);
      endError();
      return NULL;
    }
    return devices[index];
  }

  void handleInput(
    byte index, byte type, byte data1, byte data2, byte channel, byte cable = 0
  ) {

    if (handleInputRef == -1)
      handleInputRef = Lua::storeFunction("Midi", "handleInput");

    if (!Lua::getFunction(handleInputRef)) return;

    lua_pushnumber(Lua::L, index + 1); // Use one-based index.
    lua_pushnumber(Lua::L, type);
    lua_pushnumber(Lua::L, data1);
    lua_pushnumber(Lua::L, data2);
    lua_pushnumber(Lua::L, channel); // Channel is already a one-based index.
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

    AnyMidi *device = getDevice(deviceIndex);
    if (device != NULL) device->send(type, data1, data2, channel, cable);
  }

  void handleMidiClock() {
    // TODO: sync selected midi devices
    usbMIDI.sendRealTime(usbMIDI.Clock);
    TimerLib::updateEvents(currentTick, true);
    currentTick++;
    TimerLib::currentTick = currentTick;
  }

  void begin() {
    usbHost.begin();
    for (byte i = 0; i < maxDevices; i++) {
      devices[i]->begin();
      devices[i]->onInput(handleInput);
    }
    TimerLib::onEvent(handleTimerEvent);
  }

  void update() {
    usbHost.Task();
    for (byte i = 0; i < maxDevices; i++) {
      devices[i]->update();
    }
  }

  namespace lib {

    int send(lua_State *L) {
      byte index = lua_tonumber(L, 1) - 1; // Use zero-based index.
      byte type = lua_tonumber(L, 2);
      byte data1 = lua_tonumber(L, 3);
      byte data2 = lua_tonumber(L, 4);
      byte channel = lua_tonumber(L, 5); // Channel is always a one-based index.
      byte cable = lua_tonumber(L, 6) - 1; // Use zero-based index.

      AnyMidi *device = getDevice(index);
      if (device != NULL) device->send(type, data1, data2, channel, cable);
      return 0;
    }

    int parseNoteId(lua_State *L) {
      int noteId = lua_tonumber(L, 1);
      byte note = noteId & 0XFF;
      byte channel = (noteId & 0XFF00) >> 8;
      lua_pushnumber(Lua::L, note);
      lua_pushnumber(Lua::L, channel);
      return 2;
    }

    int getNoteId(lua_State *L) {
      byte note = lua_tonumber(L, 1);
      byte channel = lua_tonumber(L, 2);
      lua_pushnumber(Lua::L, ((channel & 0xFF) << 8) | (note & 0xFF));
      return 1;
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
  } // namespace lib

  void install() {
    handleInputRef = -1;
    handleClockRef = -1;

    luaL_Reg lib[] = {
      {"__send", lib::send},
      {"__getNoteId", lib::getNoteId},
      {"parseNoteId", lib::parseNoteId},
      {"start", lib::start},
      {"stop", lib::stop},
      {"setTempo", lib::setTempo},
      {NULL, NULL}};

    luaL_register(Lua::L, "Midi", lib);
  }
} // namespace MidiLib

#endif