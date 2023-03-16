#ifndef Lua_h
#define Lua_h

#include <Arduino.h>
#include <Bridge.h>
#include <FileSystem.h>
#include <Logger.h>
#include <SPI.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <sdios.h>

namespace Lua {
  using Bridge::Data;
  using Bridge::RequestId;

  typedef void (*SetupHandler)();

  lua_State *L;
  StdioStream file;
  SetupHandler handleSetup;

  void onSetup(SetupHandler handler) {
    handleSetup = handler;
  }

  bool check(int luaHasError) {
    if (luaHasError) {
      Logger::beginError();
      Logger::serial->print(lua_tostring(L, -1));
      Logger::endLog();
      lua_pop(L, 1);
    }
    return luaHasError ? false : true;
  }

  namespace {
    bool isFunction(
      const char *name, int stackIndex, bool shouldLogError = true
    ) {
      bool isFunction =
        lua_isfunction(L, stackIndex) || lua_islightfunction(L, stackIndex);

      if (!isFunction && shouldLogError) {
        Logger::beginError();
        Bridge::serial->printf(F("can't find function `%s`"), name);
        Logger::endLog();
      }

      return isFunction;
    }

    int loadFile(lua_State *L) {
      const char *fileName = lua_tostring(L, 1);
      check(luaL_loadfile(L, fileName));
      return 1;
    }

    int loadModule(lua_State *L) {
      const char *path = lua_tostring(L, 1);

      // Try to load `my/module.lua` ...
      char fileName[256];
      strncpy(fileName, path, 256);
      strncat(fileName, ".lua", 256);
      bool hasError = luaL_loadfile(L, fileName);

      // ... if it fails, try `my/module/init.lua`
      if (hasError) {
        lua_pop(L, 1); // Remove the error.

        strncpy(fileName, path, 256);
        strncat(fileName, "/init.lua", 256);
        hasError = luaL_loadfile(L, fileName);

        if (hasError) {
          lua_pop(L, 1); // Remove the error.
          return 0;
        }
      }

      return 1;
    }

    void addPolyfills() {
      lua_register(L, "loadfile", loadFile);
      lua_register(L, "loadModule", loadModule);
      check(luaL_dostring(
        L,
        "_LOADED = {}\n"
        "function require(name)\n"
        "  if _LOADED[name] == nil then\n"
        "    local root = 'lua/'\n"
        "    local path = root .. string.gsub(name, '%.', '/')\n"
        "    local module = assert(loadModule(path), 'failed to load module "
        "' .. name)\n"
        "    _LOADED[name] = module()\n"
        "  end\n"
        "  return _LOADED[name]\n"
        "end\n"
      ));
    }
  } // namespace

  bool getFunction(const char *functionName, bool shouldLogError = true) {
    lua_getglobal(L, functionName);

    if (!isFunction(functionName, -1, shouldLogError)) {
      lua_pop(L, 1); // Remove the function.
      return false;
    }

    return true;
  }

  bool getFunction(
    const char *tableName, const char *functionName, bool shouldLogError = true
  ) {
    lua_getglobal(L, tableName);
    if (!lua_istable(L, -1)) {
      if (shouldLogError) {
        Logger::beginError();
        Bridge::serial->printf(F("can't find table `%s`"), tableName);
        Logger::endLog();
      }

      lua_pop(L, 1); // Remove the table.
      return false;
    }

    lua_getfield(L, -1, functionName);
    lua_remove(L, -2); // Remove the table.

    if (!isFunction(functionName, -1, shouldLogError)) {
      lua_pop(L, 1); // Remove the function.
      return false;
    }

    return true;
  }

  int storeFunction(
    const char *tableName, const char *functionName, bool shouldLogError = true
  ) {
    if (!getFunction(tableName, functionName, shouldLogError)) return -1;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1); // Remove the function.
    return ref;
  }

  int storeFunction(const char *functionName, bool shouldLogError = true) {
    if (!getFunction(functionName, shouldLogError)) return -1;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    // Remove the function.
    lua_pop(L, 1);
    return ref;
  }

  bool getFunction(int ref, bool shouldLogError = true) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

    if (!lua_isfunction(L, -1) && !lua_islightfunction(L, -1)) {
      if (shouldLogError) {
        Logger::beginError();
        Bridge::serial->printf(F("can't find function ref `%d`"), ref);
        Logger::endLog();
      }
      lua_pop(L, 1); // Remove the function.
      return false;
    }

    return true;
  }

  void setup() {
    // Enable printf/sprintf to print floats for Teensy.
    asm(".global _printf_float");

    L = luaL_newstate();
    luaL_openlibs(L);
    lua_settop(L, 0);
    addPolyfills();

    if (handleSetup != NULL) handleSetup();
  }

  void reset() {
    if (L != NULL) lua_close(L);
    setup();
  }

  bool runFile(const char *fileName) {
    return check(luaL_dofile(L, fileName));
  }

  bool updateFile(const char *fileName) {
    bool isEntry = strcmp(fileName, "lua/init.lua") == 0;
    bool isHotReplaced = false;

    if (!isEntry && getFunction("Hmr", "update")) {
      lua_pushstring(L, fileName);
      check(lua_pcall(L, 1, 1, 0));

      isHotReplaced = lua_toboolean(L, -1);
      lua_pop(L, 1);
    }

    // If hot replacement wasn't possible we have to reload the whole
    // application.
    if (!isHotReplaced) {
      reset();
      runFile("lua/init.lua");
    }

    return isHotReplaced;
  }

  void begin() {
    setup();

    Bridge::addMethod("/lua/run", [](Data &data) {
      RequestId id = data.getInt(0);
      if (!Bridge::validateData(data, "is", 2)) return Bridge::respondError(id);

      char fileName[FileSystem::maxFileNameLength];
      data.getString(1, fileName, FileSystem::maxTempFileNameLength);

      if (!runFile(fileName)) Bridge::respondError(id);

      Bridge::respond(id);
    });

    Bridge::addMethod("/lua/update", [](Data &data) {
      RequestId id = data.getInt(0);
      if (!Bridge::validateData(data, "is", 2)) return Bridge::respondError(id);

      char fileName[FileSystem::maxFileNameLength];
      data.getString(1, fileName, FileSystem::maxTempFileNameLength);

      int isHotReplaced = updateFile(fileName);
      Bridge::respond(id, isHotReplaced);
    });
  }
} // namespace Lua

extern "C" {
void lua_compat_print(const char *string) {
  Bridge::serial->print(string);
}

int lua_compat_fopen(const char *fileName) {
  return Lua::file.fopen(fileName, "r") ? 1 : 0;
}

void lua_compat_fclose() {
  Lua::file.fclose();
}

int lua_compat_feof() {
  return Lua::file.feof();
}

size_t lua_compat_fread(void *ptr, size_t size, size_t count) {
  return Lua::file.fread(ptr, size, count);
}

int lua_compat_ferror() {
  return Lua::file.ferror();
}
}

#endif