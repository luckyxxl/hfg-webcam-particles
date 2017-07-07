#include "main.hpp"

#include "Resources.hpp"

bool Resources::create(const char *argv0) {
  rootPath = argv0;
  rootPath.erase(rootPath.find_last_of("/") + 1);
  return true;
}

void Resources::destroy() {
}

SDL_RWops *Resources::openFile(const char *filename) {
  std::string systemFileName(rootPath + filename);
  auto result = SDL_RWFromFile(systemFileName.c_str(), "rb");
  if(!result) {
    std::cout << "could not open file " << systemFileName << "\n";
  }
  return result;
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
