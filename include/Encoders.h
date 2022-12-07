#ifndef Encoders_h
#define Encoders_h

#include <Logger.h>
#include <RangeEncoder.h>

RangeEncoder encoder1(3, 2, 0, 127);
RangeEncoder encoder2(36, 37, 0, 127);
RangeEncoder encoder3(33, 34, 0, 127);

namespace Encoders {
  typedef void (*ChangeHandler)(byte encoderIndex, int32_t value);
  ChangeHandler handleChange;

  const byte maxEncoders = 3;
  uint32_t lastUpdate = 0;
  RangeEncoder *encoders[maxEncoders] = {&encoder1, &encoder2, &encoder3};

  RangeEncoder *getEncoder(byte index) {
    if (index >= maxEncoders) {
      Logger::beginError();
      // Use one-based index.
      Logger::serial->printf(F("Encoder #%d doesn't exist."), index + 1);
      Logger::endError();
      return NULL;
    }
    return encoders[index];
  }

  void update() {
    // Throttle update to once every millisecond.
    uint32_t currentTime = millis();
    if (lastUpdate == currentTime) return;

    for (byte i = 0; i < maxEncoders; i++) {
      bool changed;
      int32_t value = encoders[i]->read(changed);
      if (changed && handleChange != NULL) handleChange(i, value);
    }

    lastUpdate = currentTime;
  }

  void onChange(ChangeHandler handler) { handleChange = handler; }
} // namespace Encoders

#endif