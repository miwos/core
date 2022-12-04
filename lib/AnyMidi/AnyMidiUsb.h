#ifndef MidiUsb_h
#define MidiUsb_h

#include <AnyMidi.h>

class AnyMidiUsb : public AnyMidi {
public:
  AnyMidiUsb(byte index) : AnyMidi(index) {}

  void update() {
    if (usbMIDI.read() && handleInput != NULL) {
      byte type = usbMIDI.getType();
      byte data1 = usbMIDI.getData1();
      byte data2 = usbMIDI.getData2();
      byte channel = usbMIDI.getChannel();
      byte cable = usbMIDI.getCable();
      handleInput(index, type, data1, data2, channel, cable);
    }
  }

  void send(byte type, byte data1, byte data2, byte channel, byte cable) {
    usbMIDI.send(type, data1, data2, channel, cable);
  }
};

#endif