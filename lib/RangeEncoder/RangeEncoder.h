#ifndef RangeEncoder_h
#define RangeEncoder_h

#include <Encoder.h>

class RangeEncoder {
private:
  Encoder *encoder;
  int32_t min = 0;
  int32_t max = 127;
  int32_t value = 0;

  bool ignoreChangeOnce = false;

  int32_t constrainValue(int32_t value) {
    if (value < min) {
      value = 0;
      encoder->write(min - 1);
    } else if (value > max) {
      value = max;
      encoder->write(max + 1);
    }
    return value;
  }

public:
  RangeEncoder(uint8_t pin1, uint8_t pin2, int32_t min, int32_t max) {
    encoder = new Encoder(pin1, pin2);
    this->min = min;
    this->max = max;
  }

  void setRange(int32_t min, int32_t max) {
    this->min = min;
    this->max = max;
    value = constrainValue(value);
  }

  int32_t read() {
    value = constrainValue(encoder->read());
    return value;
  }

  int32_t read(bool &changed) {
    // If we manually write to an encoder, we don't want to trigger a change.
    // Only changes caused by the encoder itself are considered.
    if (ignoreChangeOnce) {
      changed = false;
      ignoreChangeOnce = false;
      return read();
    }

    int32_t oldValue = value;
    int32_t value = read();
    changed = value != oldValue;
    return value;
  }

  void write(int32_t value) {
    ignoreChangeOnce = true;
    encoder->write(value);
  }

  ~RangeEncoder() {
    delete encoder;
  }
};

#endif