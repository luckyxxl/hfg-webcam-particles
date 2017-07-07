#pragma once

#include "Resources.hpp"
#include "Webcam.hpp"
#include "ThreadSyncTripleBuffer.hpp"

struct Particle {
  float position[2];
  float rgb[3];
  float hsv[3];
  float localEffectStrength;
};

class Application {
  public:
  bool create(Resources *resources);
  void destroy();

  void handleEvent(const SDL_Event &event);
  void reshape(uint32_t width, uint32_t height);
  void update(float dt);
  void render();

  private:
  Webcam webcam;
  uint32_t webcam_width, webcam_height;
  ThreadSyncTripleBuffer<std::vector<float>> webcam_buffer;

  std::atomic<bool> kill_threads{false};

  void webcamThreadFunc();
  std::thread webcam_thread;

  std::vector<Particle> current_frame_data;

  GLuint program;
  GLint time_location, globalEffectTime_location;

  GLuint vertex_array;
  GLuint vertex_buffer;

  std::vector<float> background_frame;

  bool globalEffectTimeoutActive = false;
  float globalEffectTimeout = 1.f;

  bool globalEffectActive = false;
  float globalEffectTime = 0.f;
};
