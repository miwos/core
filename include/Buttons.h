#ifndef Buttons_h
#define Buttons_h

#include <Arduino.h>
#include <Button.h>

// Blue buttons
Button button1(10);
Button button2(26);
Button button3(22);

// Green buttons
Button button4(15);
Button button5(31);
Button button6(32);

// Encoder buttons
Button button7(23);
Button button8(6);
Button button9(35);

// Shift/Menu button
Button button10(1);

namespace Buttons {
  typedef void (*ClickHandler)(byte buttonIndex, uint32_t duration);
  ClickHandler handleClick;

  uint32_t throttleInterval = 5; // ms
  const byte maxButtons = 10;
  uint32_t lastUpdate = 0;

  Button *buttons[maxButtons] = {&button1, &button2, &button3, &button4,
      &button5, &button6, &button7, &button8, &button9, &button10};

  Button *getButton(byte index) {
    if (index >= maxButtons) {
      Logger::beginError();
      // Use one-based index.
      Logger::serial->printf(F("Button #%d doesn't exist."), index + 1);
      Logger::endError();
      return NULL;
    }
    return buttons[index];
  }

  void begin() {
    for (byte i = 0; i < maxButtons; i++) {
      buttons[i]->begin();
    }
  }

  void update() {
    uint32_t currentTime = millis();
    if (currentTime - lastUpdate < throttleInterval) return;

    bool clicked;
    uint32_t duration = 0;
    for (byte i = 0; i < maxButtons; i++) {
      clicked = buttons[i]->clicked(duration);
      if (clicked && handleClick != NULL) handleClick(i, duration);
    }

    lastUpdate = currentTime;
  }

  void onClick(ClickHandler handler) { handleClick = handler; }
} // namespace Buttons

#endif