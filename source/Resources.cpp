#include "main.hpp"

#include "Resources.hpp"

bool Resources::create(const char *argv0) {
  rootPath = argv0;
  rootPath.erase(rootPath.find_last_of("/") + 1);
#ifdef __APPLE__
  rootPath.append("../Resources/");
#endif
  rootPath.append("resource/");
  return true;
}

void Resources::destroy() {}

Resources::path_t Resources::resolve(const char *filename) {
  return rootPath + filename;
}

SDL_RWops *Resources::openFile(const char *filename) {
  auto systemFileName = resolve(filename);
  auto result = SDL_RWFromFile(systemFileName.c_str(), "rb");
  if (!result) {
    std::cout << "could not open file " << systemFileName << "\n";
  }
  return result;
}

std::string Resources::readWholeTextFile(const char *filename) {
  std::string result;

  std::ifstream file(resolve(filename));
  std::string line;
  while (file.good()) {
    std::getline(file, line);
    result += line + "\n";
  }

  return result;
}

std::vector<uint8_t> Resources::readWholeBinaryFile(const char *filename) {
  // https://stackoverflow.com/questions/18816126/c-read-the-whole-file-in-buffer
  std::ifstream file(resolve(filename), std::ios::binary | std::ios::ate);
  const auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> result(size);
  file.read(reinterpret_cast<char *>(result.data()), result.size());

  return result;
}
