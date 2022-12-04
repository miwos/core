#ifndef MidiSerial_h
#define MidiSerial_h

#include "AnyMidi.h"
#include <MIDI.h>

class AnyMidiSerial : public AnyMidi {
private:
  midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> *midi;

public:
  AnyMidiSerial(
      byte index, midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> *midi)
      : AnyMidi(index) {
    this->midi = midi;
  }

  void begin() {
    midi->begin(MIDI_CHANNEL_OMNI);
    midi->turnThruOff();
  }

  void update() {
    if (midi->read() && handleInput != NULL) {
      byte type = midi->getType();
      byte data1 = midi->getData1();
      byte data2 = midi->getData2();
      byte channel = midi->getChannel();
      handleInput(index, type, data1, data2, channel, 0);
    }
  }

  midi::MidiType getType(byte type) {
    switch (type) {
    case 0x80:
      return midi::MidiType::NoteOff;
    case 0x90:
      return midi::MidiType::NoteOn;
    case 0xA0:
      return midi::MidiType::NoteOff;
    case 0xB0:
      return midi::MidiType::ControlChange;
    case 0xC0:
      return midi::MidiType::ProgramChange;
    case 0xD0:
      return midi::MidiType::AfterTouchChannel;
    case 0xE0:
      return midi::MidiType::PitchBend;
    case 0xF0:
      return midi::MidiType::SystemExclusive;
    case 0xF1:
      return midi::MidiType::TimeCodeQuarterFrame;
    case 0xF2:
      return midi::MidiType::SongPosition;
    case 0xF3:
      return midi::MidiType::SongSelect;
    case 0xF6:
      return midi::MidiType::TuneRequest;
    case 0xF7:
      return midi::MidiType::SystemExclusiveEnd;
    case 0xF8:
      return midi::MidiType::Clock;
    case 0xF9:
      return midi::MidiType::Tick;
    case 0xFA:
      return midi::MidiType::Start;
    case 0xFB:
      return midi::MidiType::Continue;
    case 0xFC:
      return midi::MidiType::Stop;
    case 0xFE:
      return midi::MidiType::ActiveSensing;
    case 0xFF:
      return midi::MidiType::SystemReset;
    default:
      return midi::MidiType::InvalidType;
    };
  }

  void send(byte type, byte data1, byte data2, byte channel, byte cable) {
    midi->send(getType(type), data1, data2, channel);
  }
};

#endif