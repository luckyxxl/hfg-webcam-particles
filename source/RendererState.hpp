#pragma once

#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/Framebuffer.hpp"

struct RendererParameters {
  const graphics::Texture *particleTexture, *backgroundTexture;

  graphics::Framebuffer *outputFramebuffer;

  const uint32_t screen_width, screen_height;
  const uint32_t webcam_width, webcam_height;

  RendererParameters(graphics::Texture *particleTexture,
              graphics::Texture *backgroundTexture,
              graphics::Framebuffer *outputFramebuffer)
      : particleTexture(particleTexture), backgroundTexture(backgroundTexture),
        outputFramebuffer(outputFramebuffer),
        screen_width(outputFramebuffer->getWidth()), screen_height(outputFramebuffer->getHeight()),
        webcam_width(particleTexture->getWidth()), webcam_height(particleTexture->getHeight()) {}
};

class Clock {
public:
  void frame(float dt) {
    if (paused || period == 0.f) {
      delta = 0.f;
      return;
    }
    delta = dt;
    time += delta;
    while(time >= period) {
      if(looping) {
        time -= period;
      } else {
        time = 0.f;
        paused = true;
      }
    }
  }

  void setPeriod(float p) { period = p; }
  float getPeriod() const { return period; }

  float getTime() const { return time; }

  float getDelta() const { return delta; }

  void setPaused(bool pause) {
    if (pause != paused) {
      if (!paused) {
        delta = 0.f;
      }
      paused = pause;
    }
  }

  bool getPaused() const { return paused; }

  void setLooping(bool loop) { looping = loop; }
  bool getLooping() const { return looping; }

  // just for readability
  void play() { setPaused(false); }
  void pause() { setPaused(true); }
  bool isPlaying() { return !getPaused(); }
  bool isPaused() { return getPaused(); }
  void disableLooping() { setLooping(false); }
  void enableLooping() { setLooping(true); }

#if WITH_EDIT_TOOLS
  // really just for edit tools... really!
  float *editGetTimeP() { return &time; }
  float *editGetPeriodP() { return &period; }
  bool *editGetPausedP() { return &paused; }
#endif

private:
  float time = 0.f;
  float delta = 0.f;
  float period = 1000.f;
  bool paused = false;
  bool looping = true;
};

struct RendererState {
  Clock clock;
};

struct RenderProps : RendererParameters {
  const RendererState &state;

  std::default_random_engine &random;

  RenderProps(const RendererParameters &parameters, const RendererState &state,
              std::default_random_engine &random)
              : RendererParameters(parameters), state(state), random(random) {}
};
