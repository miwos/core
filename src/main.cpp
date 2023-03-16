#include <Arduino.h>
#include <Bridge.h>

#include <modules/Bridge.h>
#include <modules/Buttons.h>
#include <modules/Displays.h>
#include <modules/Encoders.h>
#include <modules/FileSystem.h>
#include <modules/Leds.h>
#include <modules/Log.h>
#include <modules/Midi.h>
#include <modules/Timer.h>
#include <modules/Utils.h>

#include <FileSystem.h>
#include <SlipSerial.h>
#include <helpers/Lua.h>

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
    ModuleBridge::API::install();
    Buttons::API::install();
    Displays::API::install();
    Encoders::API::install();
    MyFileSystem::API::install();
    Leds::API::install();
    Log::API::install();
    Midi::API::install();
    Timer::API::install();
    Utils::API::install();
  });

  Bridge::begin(serial);
  Buttons::begin();
  Displays::begin();
  FileSystem::begin();
  Leds::begin();
  Midi::begin();

  Lua::begin();
  ModuleBridge::begin();

  // Prevent auto-running `init.lua` by holding down the menu button when
  // powering on the device. This is useful for debugging, for example if there
  // is an infinite loop in `init.lua` that would cause the device to freeze
  // immediately at startup.
  if (!Buttons::buttons[9].read()) Lua::runFile("lua/init.lua");

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
  Midi::update();
  Timer::update();
  Displays::update();

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