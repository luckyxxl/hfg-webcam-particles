#pragma once

#include "Resources.hpp"
#include "Webcam.hpp"

class Application {
  public:
  Application(Resources *resources, Webcam *webcam);
  ~Application();

  void handleEvent(const SDL_Event &event);
  void reshape(uint32_t width, uint32_t height);
  void update(float dt);
  void render();

  private:
  Webcam *webcam;
  uint32_t webcam_width, webcam_height;
  std::vector<float> webcam_frame;

  GLuint program;

  GLuint vertex_array;
  GLuint vertex_buffer;
};
