#pragma once

namespace graphics {

class Window {
public:
  bool create();
  void destroy();

  void swap();
  bool toggleFullscreen();

  std::tuple<uint32_t, uint32_t> getSize() const;

private:
  SDL_Window *window = nullptr;
  SDL_GLContext gl_context = nullptr;

  struct {
    uint32_t lastTime = 0u;
    uint32_t counter = 0u;
  } fps;

  void updateFPS();
};
} // namespace graphics
