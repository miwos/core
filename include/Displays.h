#ifndef Displays_h
#define Displays_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Logger.h>
#include <fonts/vevey_pixel_12pt.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK
#define OLED_RESET 4

namespace Displays {
  typedef Adafruit_SSD1306 Display;
  const byte maxDisplays = 3;

  Display displays[maxDisplays] = {
      Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET),
      Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, OLED_RESET)};

  void initializeDisplay(Display *display) {
    display->clearDisplay();
    display->setFont(&vevey_pixel_12pt);
    display->setTextColor(WHITE);
  }

  void begin() {
    for (byte i = 0; i < maxDisplays; i++) {
      if (displays[i].begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        initializeDisplay(&displays[i]);
      } else {
        Logger::beginError();
        // Use one-based index.
        Logger::serial->printf(
            F("Display #%d couldn't be initialized."), i + 1);
        Logger::endError();
      }
    }
  }

  Display *getDisplay(byte index) {
    if (index >= maxDisplays) {
      Logger::beginError();
      // Use one-based index.
      Logger::serial->printf(F("Display #%d doesn't exist."), index + 1);
      Logger::endError();
      return NULL;
    }
    return &(displays[index]);
  }

} // namespace Displays

#endif