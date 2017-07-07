#pragma once

#include "Resources.hpp"
#include "Webcam.hpp"
#include "ThreadSyncTripleBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "graphics/ParticleBuffer.hpp"

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

  graphics::Pipeline pipeline;
  GLint time_location, globalEffectTime_location;

  // remove this and use glMapBuffer?  thought mapping is probably slower...
  std::vector<graphics::Particle> current_frame_data;
  graphics::ParticleBuffer particleBuffer;

  std::vector<float> background_frame;

  bool globalEffectTimeoutActive = false;
  float globalEffectTimeout = 1.f;

  bool globalEffectActive = false;
  float globalEffectTime = 0.f;
};
