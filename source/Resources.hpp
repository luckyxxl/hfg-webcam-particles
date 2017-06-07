#pragma once

class Resources {
  public:
  Resources(const char *argv0);
  ~Resources();

  std::string readWholeFile(const char *filename);

  private:
  std::string rootPath;
};
