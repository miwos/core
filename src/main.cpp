#include <Arduino.h>
#include <Bridge.h>
#include <Buttons.h>
#include <Displays.h>
#include <Encoders.h>
#include <FileSystem.h>
#include <Leds.h>
#include <Lua.h>
#include <LuaBridge.h>
#include <LuaButtons.h>
#include <LuaDisplays.h>
#include <LuaEncoders.h>
#include <LuaFileSystem.h>
#include <LuaLeds.h>
#include <LuaLog.h>
#include <LuaMidi.h>
#include <LuaTimer.h>
#include <MidiDevices.h>
#include <SlipSerial.h>

using Bridge::Data;
using Bridge::RequestId;

SlipSerial serial(Serial);

void setup() {
  Serial.begin(9600);
  // while (!Serial) {
  // }

  Lua::onSetup([]() {
    LuaBridge::install();
    LuaButtons::install();
    LuaDisplays::install();
    LuaEncoders::install();
    LuaFileSystem::install();
    LuaLeds::install();
    LuaLog::install();
    LuaMidi::install();
    LuaTimer::install();
  });

  Bridge::begin(serial);
  Buttons::begin();
  Displays::begin();
  FileSystem::begin();
  Leds::begin();
  MidiDevices::begin();

  Lua::begin();
  LuaBridge::begin();
  LuaButtons::begin();
  LuaEncoders::begin();
  LuaMidi::begin();

  // Prevent auto-running `init.lua` by holding down the menu button when
  // powering on the device. This is useful for debugging, for example if there
  // is an infinite loop in `init.lua` that would cause the device to freeze
  // immediately at startup.
  if (!Buttons::buttons[9]->read()) Lua::runFile("lua/init.lua");

  // Simple echo, useful for debugging bridge communication between the miwos
  // app and device.
  Bridge::addMethod("/echo/int", [](Data &data) {
    RequestId id = data.getInt(0);
    if (!Bridge::validateData(data, "ii", 2)) return Bridge::respondError(id);
    auto number = data.getInt(1);
    Bridge::respond(id, number);
  });
}

void loop() {
  Bridge::update();
  Buttons::update();
  Encoders::update();
  MidiDevices::update();
  LuaTimer::update();
}