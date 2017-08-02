#pragma once

class Resources {
public:
  bool create(const char *argv0);
  void destroy();

  SDL_RWops *openFile(const char *filename);

  std::string readWholeTextFile(const char *filename);
  std::vector<uint8_t> readWholeBinaryFile(const char *filename);

private:
  std::string rootPath;
};
