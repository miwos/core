#ifndef Leds_h
#define Leds_h

#include <Arduino.h>
#include <Logger.h>

namespace Leds {
  const byte ON = 255;
  const byte OFF = 0;
  const byte maxLeds = 7;

  struct Led {
    byte pin;
    bool pwm;
  };

  Led leds[maxLeds] = {
      {9, true},
      {11, true},
      {27, false},
      {14, true},
      {30, false},
      {29, true},
      {0, true},
  };

  byte pwmResolution = 8;
  float pwmFrequency = 585937.5;
  byte pins[maxLeds] = {9, 11, 27, 14, 30, 29, 0};

  void write(byte index, byte intensity) {
    if (index >= maxLeds) {
      Logger::beginError();
      // Increase the index to be consistent with lua's index.
      Logger::serial->printf(F("LED #%d doesn't exist."), index + 1);
      Logger::endError();
      return;
    }
    byte pin = leds[index].pin;
    if (leds[index].pwm) {
      analogWrite(pin, intensity);
    } else {
      digitalWrite(pin, intensity == 0 ? LOW : HIGH);
    }
  }

  void begin() {
    analogWriteResolution(pwmResolution);
    for (byte i = 0; i < maxLeds; i++) {
      byte pin = leds[i].pin;
      pinMode(pin, OUTPUT);
      if (leds[i].pwm) analogWriteFrequency(pin, pwmFrequency);
      write(i, 0);
    }
  }
}; // namespace Leds

#endif