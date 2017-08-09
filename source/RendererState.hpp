#pragma once

#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/ParticleBuffer.hpp"
#include "graphics/Framebuffer.hpp"

struct RendererParameters {
  const graphics::ScreenRectBuffer &screen_rect_buffer;
  const graphics::ParticleBuffer &particle_buffer;
  graphics::Framebuffer &particle_fb, &accumulation_fb, &result_fb;

  std::default_random_engine &random;

  const uint32_t &screen_width, &screen_height;
  const uint32_t &webcam_width, &webcam_height;

  RendererParameters(const graphics::ScreenRectBuffer &screen_rect_buffer,
              const graphics::ParticleBuffer &particle_buffer, std::default_random_engine &random,
              graphics::Framebuffer &particle_fb, graphics::Framebuffer &accumulation_fb, graphics::Framebuffer &result_fb,
              const uint32_t &screen_width, const uint32_t &screen_height,
              const uint32_t &webcam_width, const uint32_t &webcam_height)
      : screen_rect_buffer(screen_rect_buffer), particle_buffer(particle_buffer), particle_fb(particle_fb), accumulation_fb(accumulation_fb), result_fb(result_fb),
        random(random), screen_width(screen_width), screen_height(screen_height),
        webcam_width(webcam_width), webcam_height(webcam_height) {}
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

  RenderProps(const RendererParameters &parameters, const RendererState &state) : RendererParameters(parameters), state(state) {}
};
