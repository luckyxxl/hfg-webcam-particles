#pragma once

class Resources {
  public:
  bool create(const char *argv0);
  void destroy();

  SDL_RWops *openFile(const char *filename);

  std::string readWholeFile(const char *filename);

  private:
  std::string rootPath;
};
