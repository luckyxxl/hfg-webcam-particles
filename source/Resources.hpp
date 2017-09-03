#pragma once
#include <SDL.h>

class Resources {
public:
  using path_t = std::string;

  bool create(const char *argv0);
  void destroy();

  path_t resolve(const char *filename);
  SDL_RWops *openFile(const char *filename);

  std::string readWholeTextFile(const char *filename);
  std::vector<uint8_t> readWholeBinaryFile(const char *filename);

private:
  path_t rootPath;
};
