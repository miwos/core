#ifndef LuaFileSystemLib_h
#define LuaFileSystemLib_h

#include <FileSystem.h>
#include <helpers/Lua.h>

namespace FileSystemLib {
  using FileSystem::sd;

  char fileName[FileSystem::maxFileNameLength];

  void begin() {
    FileSystem::begin();
  }

  namespace lib {
    int listFiles(lua_State *L) {
      const char *dirName = luaL_checkstring(L, 1);

      FatFile dir;
      FatFile file;

      if (!dir.open(dirName))
        return luaL_error(L, "failed to open dir %s", dirName);

      if (!dir.isDir()) return 0;
      dir.rewind();

      lua_newtable(L);
      int i = 0;
      while (file.openNext(&dir, O_READ)) {
        if (!file.isHidden() && !file.isDir()) {
          file.getName(fileName, sizeof(fileName));
          lua_pushstring(L, fileName);
          lua_rawseti(L, -2, i + 1);
          i++;
        }
        file.close();
      }

      dir.close();
      return 1;
    }

    int fileExists(lua_State *L) {
      const char *fileName = luaL_checkstring(L, 1);
      lua_pushboolean(L, sd.exists(fileName));
      return 1;
    }

    int writeFile(lua_State *L) {
      const char *fileName = luaL_checkstring(L, 1);
      const char *content = luaL_checkstring(L, 2);
      bool overwrite = lua_toboolean(L, 3);
      FatFile file;

      if (overwrite && sd.exists(fileName)) sd.remove(fileName);
      file.open(fileName, FILE_WRITE);
      int result = file.write(content);
      file.close();

      lua_pushboolean(L, result > -1);
      return 1;
    }
  } // namespace lib

  void install() {
    luaL_Reg lib[] = {
      {"listFiles", lib::listFiles},
      {"fileExists", lib::fileExists},
      {"writeFile", lib::writeFile},
      {NULL, NULL}};
    luaL_register(Lua::L, "FileSystem", lib);
  }
} // namespace FileSystemLib

#endif