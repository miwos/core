#ifndef MidiUsbHub_h
#define MidiUsbHub_h

#include "AnyMidi.h"
#include <USBHost_t36.h>

class AnyMidiUsbHub : public AnyMidi {
private:
  MIDIDevice *midi;

public:
  AnyMidiUsbHub(byte index, MIDIDevice *midi) : AnyMidi(index) {
    this->midi = midi;
  }

  void update() {
    if (midi->read() && handleInput != NULL) {
      byte type = midi->getType();
      byte data1 = midi->getData1();
      byte data2 = midi->getData2();
      byte channel = midi->getChannel();
      byte cable = midi->getCable();
      handleInput(index, type, data1, data2, channel, cable);
    }
  }

  void send(byte type, byte data1, byte data2, byte channel, byte cable) {
    midi->send(type, data1, data2, channel, cable);
  }
};

#endif