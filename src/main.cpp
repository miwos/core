#include <Arduino.h>
#include <Bridge.h>
#include <FileSystem.h>
#include <Lua.h>
#include <LuaBridge.h>
#include <LuaFileSystem.h>
#include <LuaLog.h>
#include <LuaTimer.h>
#include <SlipSerial.h>

using Bridge::Data;
using Bridge::RequestId;

SlipSerial serial(Serial);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }

  Lua::onSetup([]() {
    LuaBridge::install();
    LuaFileSystem::install();
    LuaTimer::install();
    LuaLog::install();
  });

  Bridge::begin(serial);
  LuaBridge::begin();
  FileSystem::begin();
  Lua::begin();

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
  LuaTimer::update();
}