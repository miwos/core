#ifndef LuaDisplay_h
#define LuaDisplay_h

#include <Displays.h>
#include <Lua.h>

namespace LuaDisplays {
  using Displays::Display;

  int text(lua_State *L) {
    byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
    const char *text = luaL_checkstring(L, 2);
    byte color = luaL_checknumber(L, 3);

    Display *display = Displays::getDisplay(index);
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

    Display *display = Displays::getDisplay(index);
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

    Display *display = Displays::getDisplay(index);
    if (display == NULL) return 0;

    if (x0 == x1)
      display->drawFastVLine(x0, y0, abs(y1 - y0), color);
    else if (y0 == y1)
      display->drawFastHLine(x0, y0, abs(x1 - x0), color);
    else
      display->drawLine(x0, y0, x1, y1, color);

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

    Display *display = Displays::getDisplay(index);
    if (display == NULL) return 0;

    if (fill)
      display->fillTriangle(x0, y0, x1, y1, x2, y2, color);
    else
      display->drawTriangle(x0, y0, x1, y1, x2, y2, color);

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

    Display *display = Displays::getDisplay(index);
    if (display == NULL) return 0;

    if (fill)
      display->fillRect(x, y, width, height, color);
    else
      display->drawRect(x, y, width, height, color);

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

    Display *display = Displays::getDisplay(index);
    if (display == NULL) return 0;

    if (fill)
      display->fillRoundRect(x, y, width, height, radius, color);
    else
      display->drawRoundRect(x, y, width, height, radius, color);

    return 0;
  }

  int drawCircle(lua_State *L) {
    byte index = luaL_checknumber(L, 1) - 1; // Use zero-based index.
    byte x = luaL_checknumber(L, 2);
    byte y = luaL_checknumber(L, 3);
    byte radius = luaL_checknumber(L, 4);
    byte color = luaL_checknumber(L, 5);
    bool fill = lua_toboolean(L, 6);

    Display *display = Displays::getDisplay(index);
    if (display == NULL) return 0;

    if (fill)
      display->fillCircle(x, y, radius, color);
    else
      display->drawCircle(x, y, radius, color);

    return 0;
  }

  int update(lua_State *L) {
    byte index = lua_tonumber(L, 1) - 1; // Use zero-based index.
    Display *display = Displays::getDisplay(index);
    if (display != NULL) display->display();
    return 0;
  }

  int clear(lua_State *L) {
    byte index = lua_tonumber(L, 1) - 1; // Use zero-based index.
    Display *display = Displays::getDisplay(index);
    if (display != NULL) display->clearDisplay();
    return 0;
  }

  void install() {
    luaL_Reg lib[] = {{"text", text}, {"drawPixel", drawPixel},
        {"drawLine", drawLine}, {"drawTriangle", drawTriangle},
        {"drawRectangle", drawRectangle},
        {"drawRoundedRectangle", drawRoundedRectangle},
        {"drawCircle", drawCircle}, {"update", update}, {"clear", clear},
        {NULL, NULL}};
    luaL_register(Lua::L, "Displays", lib);
  }

} // namespace LuaDisplays

#endif