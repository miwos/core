#ifndef MiwosDisplaysLib_h
#define MiwosDisplaysLib_h

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Logger.h>
#include <fonts/vevey_pixel_12pt.h>
#include <helpers/Lua.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK
#define OLED_RESET 4

namespace DisplaysLib {
  using Logger::beginError;
  using Logger::endError;
  using Logger::serial;

  typedef Adafruit_SSD1306 Display;
  const byte maxDisplays = 3;
  Display displays[maxDisplays] = {
    Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET),
    Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, OLED_RESET)};

  int needsUpdate[maxDisplays] = {false};
  // A display update is ~12ms. If all three displays need an update 24fps
  // would still leave enough time to draw all of them.
  byte fps = 24;
  uint32_t frameDuration = 1000 / fps; // ms
  uint32_t lastFrameTime = 0;

  void initializeDisplay(Display *display) {
    display->clearDisplay();
    display->setFont(&vevey_pixel_12pt);
    display->setTextColor(WHITE);
    display->display();
  }

  Display *getDisplay(byte index) {
    if (index >= maxDisplays) {
      beginError();
      // Use one-based index.
      serial->printf(F("Display #%d doesn't exist."), index + 1);
      endError();
      return NULL;
    }
    return &(displays[index]);
  }

  void begin() {
    for (byte i = 0; i < maxDisplays; i++) {
      if (displays[i].begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        initializeDisplay(&displays[i]);
      } else {
        beginError();
        // Use one-based index.
        serial->printf(F("Display #%d couldn't be initialized."), i + 1);
        endError();
      }
    }
  }

  void update() {
    uint32_t now = millis();
    if (now - lastFrameTime >= frameDuration) {
      for (int i = 0; i < maxDisplays; i++) {
        if (!needsUpdate[i]) continue;

        Display *display = getDisplay(i);
        if (display != NULL) display->display();

        needsUpdate[i] = false;
      }
      lastFrameTime = now;
    }
  }

  namespace lib {
    int text(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      const char *text = luaL_checkstring(L, 2);
      byte color = luaL_checknumber(L, 3);

      Display *display = getDisplay(index);
      if (display == NULL) return 0;

      display->setCursor(0, 17);
      display->setTextColor(color);
      display->print(text);
      return 0;
    }

    int drawPixel(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte x = luaL_checknumber(L, 2);
      byte y = luaL_checknumber(L, 3);
      byte color = luaL_checknumber(L, 4);

      Display *display = getDisplay(index);
      if (display != NULL) display->drawPixel(x, y, color);
      return 0;
    }

    int drawLine(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte x0 = luaL_checknumber(L, 2);
      byte y0 = luaL_checknumber(L, 3);
      byte x1 = luaL_checknumber(L, 4);
      byte y1 = luaL_checknumber(L, 5);
      byte color = luaL_checknumber(L, 6);

      Display *display = getDisplay(index);
      if (display == NULL) return 0;

      if (x0 == x1) {
        display->drawFastVLine(x0, y0, abs(y1 - y0), color);
      } else if (y0 == y1) {
        display->drawFastHLine(x0, y0, abs(x1 - x0), color);
      } else {
        display->drawLine(x0, y0, x1, y1, color);
      }
      return 0;
    }

    int drawTriangle(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte x0 = luaL_checknumber(L, 2);
      byte y0 = luaL_checknumber(L, 3);
      byte x1 = luaL_checknumber(L, 4);
      byte y1 = luaL_checknumber(L, 5);
      byte x2 = luaL_checknumber(L, 6);
      byte y2 = luaL_checknumber(L, 7);
      byte color = luaL_checknumber(L, 8);
      bool fill = lua_toboolean(L, 9);

      Display *display = getDisplay(index);
      if (display == NULL) return 0;

      if (fill) {
        display->fillTriangle(x0, y0, x1, y1, x2, y2, color);
      } else {
        display->drawTriangle(x0, y0, x1, y1, x2, y2, color);
      }
      return 0;
    }

    int drawRectangle(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte x = luaL_checknumber(L, 2);
      byte y = luaL_checknumber(L, 3);
      byte width = luaL_checknumber(L, 4);
      byte height = luaL_checknumber(L, 5);
      byte color = luaL_checknumber(L, 6);
      bool fill = lua_toboolean(L, 7);

      Display *display = getDisplay(index);
      if (display == NULL) return 0;

      if (fill) {
        display->fillRect(x, y, width, height, color);
      } else {
        display->drawRect(x, y, width, height, color);
      }
      return 0;
    }

    int drawRoundedRectangle(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte x = luaL_checknumber(L, 2);
      byte y = luaL_checknumber(L, 3);
      byte width = luaL_checknumber(L, 4);
      byte height = luaL_checknumber(L, 5);
      byte radius = luaL_checknumber(L, 6);
      byte color = luaL_checknumber(L, 7);
      bool fill = lua_toboolean(L, 8);

      Display *display = getDisplay(index);
      if (display == NULL) return 0;

      if (fill) {
        display->fillRoundRect(x, y, width, height, radius, color);
      } else {
        display->drawRoundRect(x, y, width, height, radius, color);
      }
      return 0;
    }

    int drawCircle(lua_State *L) {
      byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
      byte x = luaL_checknumber(L, 2);
      byte y = luaL_checknumber(L, 3);
      byte radius = luaL_checknumber(L, 4);
      byte color = luaL_checknumber(L, 5);
      bool fill = lua_toboolean(L, 6);

      Display *display = getDisplay(index);
      if (display == NULL) return 0;

      if (fill) {
        display->fillCircle(x, y, radius, color);
      } else {
        display->drawCircle(x, y, radius, color);
      }
      return 0;
    }

    int update(lua_State *L) {
      byte index = lua_tonumber(L, 1) - 1; // Use zero-based index.
      if (getDisplay(index) != NULL) needsUpdate[index] = true;
      return 0;
    }

    int clear(lua_State *L) {
      byte index = lua_tonumber(L, 1) - 1; // Use zero-based index.
      Display *display = getDisplay(index);
      if (display != NULL) display->clearDisplay();
      return 0;
    }
  } // namespace lib

  void install() {
    luaL_Reg lib[] = {
      {"text", lib::text},
      {"drawPixel", lib::drawPixel},
      {"drawLine", lib::drawLine},
      {"drawTriangle", lib::drawTriangle},
      {"drawRectangle", lib::drawRectangle},
      {"drawRoundedRectangle", lib::drawRoundedRectangle},
      {"drawCircle", lib::drawCircle},
      {"update", lib::update},
      {"clear", lib::clear},
      {NULL, NULL}};

    luaL_register(Lua::L, "Displays", lib);
  }
} // namespace DisplaysLib

#endif