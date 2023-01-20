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

#if defined(DEBUG) && defined(DEBUG_LOOP)
uint32_t lastLoopTime = 0;
uint32_t maxLoopInterval = 0;
uint32_t lastLoopLogTime = 0;
uint32_t loopLogInterval = 1000000;
#endif

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
  LuaDisplays::update();

#if defined(DEBUG) && defined(DEBUG_LOOP)
  uint32_t now = micros();
  maxLoopInterval = max(maxLoopInterval, now - lastLoopTime);

  if (now - lastLoopLogTime >= loopLogInterval) {
    const char *color = maxLoopInterval < 100   ? "success"
                        : maxLoopInterval < 500 ? "warn"
                                                : "error";

    Logger::beginInfo();
    Logger::serial->printf(
        F("{gray Loop interval:} {%s %dμs}\n"), color, maxLoopInterval);
    Logger::endInfo();
    lastLoopLogTime = now;
    maxLoopInterval = 0;
  }

  lastLoopTime = now;
#endif
}