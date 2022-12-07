#include <Arduino.h>
#include <Bridge.h>
#include <Displays.h>
#include <Encoders.h>
#include <FileSystem.h>
#include <Lua.h>
#include <LuaBridge.h>
#include <LuaDisplays.h>
#include <LuaEncoders.h>
#include <LuaFileSystem.h>
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

  Lua::onSetup([]() {
    LuaBridge::install();
    LuaDisplays::install();
    LuaEncoders::install();
    LuaFileSystem::install();
    LuaLog::install();
    LuaMidi::install();
    LuaTimer::install();
  });

  Bridge::begin(serial);
  Displays::begin();
  FileSystem::begin();
  MidiDevices::begin();

  Lua::begin();
  LuaBridge::begin();
  LuaEncoders::begin();
  LuaMidi::begin();

  Lua::runFile("lua/init.lua");

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
  Encoders::update();
  MidiDevices::update();
  LuaTimer::update();
}