#pragma once

class Resources {
  public:
  bool create(const char *argv0);
  void destroy();

  std::string readWholeFile(const char *filename);

  private:
  std::string rootPath;
};
