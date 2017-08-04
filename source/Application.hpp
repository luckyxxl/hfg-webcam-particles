#pragma once

#include "ParticleRenderer.hpp"
#include "ThreadSyncTripleBuffer.hpp"
#include "Timeline.hpp"
#include "Webcam.hpp"
#include "effects/EffectRegistry.hpp"
#include "graphics/ParticleBuffer.hpp"
#include "graphics/Pipeline.hpp"
#include "sound/SampleBuffer.hpp"

class Resources;
namespace sound {
class Renderer;
}
namespace graphics {
class Window;
}

class Application {
public:
  bool create(Resources *, graphics::Window *, sound::Renderer *);
  void destroy();

  bool handleEvents();
  void reshape(uint32_t width, uint32_t height);
  void update(float dt);
  void render();

private:
  graphics::Window *window;
  sound::Renderer *soundRenderer;

  EffectRegistry effectRegistry;

  sound::SampleBuffer backgroundLoop;
  std::vector<sound::SampleBuffer> whooshSamples;

  std::default_random_engine random;

  uint32_t screen_width, screen_height;

  Webcam webcam;
  uint32_t webcam_width, webcam_height;
  ThreadSyncTripleBuffer<std::vector<float>> webcam_buffer;

  std::atomic<bool> kill_threads{false};

  void webcamThreadFunc();
  std::thread webcam_thread;

  ParticleRenderer standbyParticleRenderer;
  ParticleRenderer reactionParticleRenderer;

  enum class ReactionState {
    Inactive,
    FinishStandbyTimeline,
    RenderReactionTimeline,
  } reactionState = ReactionState::Inactive;

  // remove this and use glMapBuffer?  thought mapping is probably slower...
  std::vector<graphics::Particle> current_frame_data;
  graphics::ParticleBuffer particleBuffer;

  std::vector<float> background_frame;
};
