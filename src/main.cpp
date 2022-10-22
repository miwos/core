#include <Arduino.h>
#include <Bridge.h>
#include <FileSystem.h>
#include <Lua.h>
#include <LuaFileSystem.h>
#include <LuaLog.h>
#include <LuaTimer.h>
#include <SlipSerial.h>

SlipSerial serial(Serial);

int ref;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }

  Bridge::begin(serial);
  FileSystem::begin();
  Lua::begin();

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