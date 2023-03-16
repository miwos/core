#include <Arduino.h>
#include <Bridge.h>
#include <FileSystem.h>
#include <SlipSerial.h>
#include <helpers/Lua.h>
#include <lua/BridgeLib.h>
#include <lua/ButtonsLib.h>
#include <lua/DisplaysLib.h>
#include <lua/EncodersLib.h>
#include <lua/FileSystemLib.h>
#include <lua/LedsLib.h>
#include <lua/LogLib.h>
#include <lua/MidiLib.h>
#include <lua/TimerLib.h>
#include <lua/UtilsLib.h>

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
  // Activate this if you don't want to miss any serial when debugging:
  // while (!Serial) {}

  Lua::onSetup([]() {
    BridgeLib::install();
    ButtonsLib::install();
    DisplaysLib::install();
    EncodersLib::install();
    FileSystemLib::install();
    LedsLib::install();
    LogLib::install();
    MidiLib::install();
    TimerLib::install();
    UtilsLib::install();
  });

  Bridge::begin(serial);
  Lua::begin();

  ButtonsLib::begin();
  DisplaysLib::begin();
  FileSystemLib::begin();
  LedsLib::begin();
  MidiLib::begin();
  BridgeLib::begin();

  // Prevent auto-running `init.lua` by holding down the menu button when
  // powering on the device. This is useful for debugging, for example if there
  // is an infinite loop in `init.lua` that would cause the device to freeze
  // immediately at startup.
  if (!ButtonsLib::buttons[9].read()) Lua::runFile("lua/init.lua");

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
  ButtonsLib::update();
  EncodersLib::update();
  MidiLib::update();
  TimerLib::update();
  DisplaysLib::update();

#if defined(DEBUG) && defined(DEBUG_LOOP)
  uint32_t now = micros();
  maxLoopInterval = max(maxLoopInterval, now - lastLoopTime);

  if (now - lastLoopLogTime >= loopLogInterval) {
    const char *color = maxLoopInterval < 100 ? "success"
      : maxLoopInterval < 500                 ? "warn"
                                              : "error";

    Logger::beginInfo();
    Logger::serial->printf(
      F("{gray Loop interval:} {%s %dμs}\n"), color, maxLoopInterval
    );
    Logger::endInfo();
    lastLoopLogTime = now;
    maxLoopInterval = 0;
  }

  lastLoopTime = now;
#endif
}