#ifndef Button_h
#define Button_h

class Button {
private:
  byte pin;
  bool state = false;
  uint32_t lastPressed = 0;

public:
  enum Type { ActiveLow, ActiveHigh };
  Type type = Type::ActiveLow;

  Button(byte pin) { this->pin = pin; }

  void begin() { pinMode(pin, INPUT_PULLUP); }

  bool read() {
    bool value = digitalRead(pin);
    return type == Type::ActiveHigh ? value : !value;
  }

  bool read(bool &changed) {
    bool oldState = state;
    state = read();
    changed = state != oldState;
    return state;
  }

  bool clicked(uint32_t &duration) {
    uint32_t currentTime = millis();

    bool changed;
    read(changed);

    bool clicked = false;
    if (changed) {
      if (state) {
        // Button was pressed.
        lastPressed = currentTime;
      } else {
        // Button was released.
        duration = currentTime - lastPressed;
        clicked = true;
      }
    }

    return clicked;
  }
};

#endif