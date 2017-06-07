#include "main.hpp"

#include "Resources.hpp"

Resources::Resources(const char *argv0) {
  rootPath = argv0;
  rootPath.erase(rootPath.find_last_of("/") + 1);
}

Resources::~Resources() {
}

std::string Resources::readWholeFile(const char *filename) {
  std::string result;

  std::ifstream file(rootPath + filename);
  std::string line;
  while(file.good()) {
    std::getline(file, line);
    result += line + "\n";
  }

  return result;
}
