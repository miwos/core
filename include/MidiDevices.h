#ifndef MidiDevices_h
#define MidiDevices_h

#include <AnyMidi.h>
#include <AnyMidiSerial.h>
#include <AnyMidiUsb.h>
#include <AnyMidiUsbHub.h>
#include <Logger.h>
#include <LuaMidi.h>
#include <MIDI.h>
#include <SPI.h>
#include <USBHost_t36.h>

// Midi via USB/Power port.
AnyMidiUsb midiDevice1(0);

// Hardware Midi
MIDI_CREATE_INSTANCE(HardwareSerial, Serial5, serialMidi1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, serialMidi2);
AnyMidiSerial midiDevice2(1, &serialMidi1);
AnyMidiSerial midiDevice3(2, &serialMidi2);

// Usb Host Midi
USBHost usbHost;
USBHub hub1(usbHost);
USBHub hub2(usbHost);
USBHub hub3(usbHost);
USBHub hub4(usbHost);
MIDIDevice hubMidi1(usbHost);
MIDIDevice hubMidi2(usbHost);
MIDIDevice hubMidi3(usbHost);
MIDIDevice hubMidi4(usbHost);
MIDIDevice hubMidi5(usbHost);
MIDIDevice hubMidi6(usbHost);
MIDIDevice hubMidi7(usbHost);
MIDIDevice hubMidi8(usbHost);
MIDIDevice hubMidi9(usbHost);
MIDIDevice hubMidi10(usbHost);
AnyMidiUsbHub midiDevice4(3, &hubMidi1);
AnyMidiUsbHub midiDevice5(4, &hubMidi2);
AnyMidiUsbHub midiDevice6(5, &hubMidi3);
AnyMidiUsbHub midiDevice7(6, &hubMidi4);
AnyMidiUsbHub midiDevice8(7, &hubMidi5);
AnyMidiUsbHub midiDevice9(8, &hubMidi6);
AnyMidiUsbHub midiDevice10(9, &hubMidi7);
AnyMidiUsbHub midiDevice11(10, &hubMidi8);
AnyMidiUsbHub midiDevice12(11, &hubMidi9);
AnyMidiUsbHub midiDevice13(12, &hubMidi10);

namespace MidiDevices {
  const byte maxDevices = 13;
  AnyMidi *devices[maxDevices] = {&midiDevice1, &midiDevice2, &midiDevice3,
      &midiDevice4, &midiDevice5, &midiDevice6, &midiDevice7, &midiDevice8,
      &midiDevice9, &midiDevice10, &midiDevice11, &midiDevice12, &midiDevice13};

  AnyMidi *getDevice(byte index) {
    if (index >= maxDevices) {
      Logger::beginError();
      // Use one-based index.
      Logger::serial->printf(F("Midi device #%d not found"), index + 1);
      Logger::endError();
      return NULL;
    }
    return devices[index];
  }

  void begin() {
    usbHost.begin();
    for (byte i = 0; i < maxDevices; i++) {
      devices[i]->begin();
    }
  }

  void update() {
    usbHost.Task();
    for (byte i = 0; i < maxDevices; i++) {
      devices[i]->update();
    }
  }
} // namespace MidiDevices

#endif