#pragma once

#include "Resources.hpp"
#include "Webcam.hpp"
#include "ThreadSyncTripleBuffer.hpp"

struct Particle {
  float position[2];
  float rgb[3];
  float hsv[3];
};

class Application {
  public:
  Application(Resources *resources);
  ~Application();

  void handleEvent(const SDL_Event &event);
  void reshape(uint32_t width, uint32_t height);
  void update(float dt);
  void render();

  private:
  std::atomic<bool> kill_threads;

  Webcam *webcam;
  uint32_t webcam_width, webcam_height;

  void webcamThreadFunc();
  std::thread webcam_thread;
  ThreadSyncTripleBuffer<std::vector<float>> webcam_buffer;

  std::vector<Particle> current_frame_data;

  GLuint program;
  GLint time_location;

  GLuint vertex_array;
  GLuint vertex_buffer;
};
