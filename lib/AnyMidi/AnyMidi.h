#ifndef AnyMidi_h
#define AnyMidi_h

#include <Arduino.h>

class AnyMidi {
public:
  byte index;
  typedef void (*InputHandler)(
      byte index, byte type, byte data1, byte data2, byte channel, byte cable);
  InputHandler handleInput;

  AnyMidi(byte index) { this->index = index; }

  virtual void begin(){};

  virtual void update() = 0;

  virtual void send(
      byte type, byte data1, byte data2, byte channel, byte cable) = 0;

  void onInput(InputHandler handler) { handleInput = handler; }
};

#endif