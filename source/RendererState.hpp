#pragma once

class Clock {
public:
  void frame(float dt) {
    if (paused || period == 0.f) {
      delta = 0.f;
      return;
    }
    if (time == -1.f) {
      time = 0.f;
    } else {
      delta = dt;
      time += delta;
      while (time >= period) {
        time -= period;
      }
    }
  }

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

private:
  float time = -1.f;
  float delta = 0.f;
  float period = 1000.f;
  bool paused = false;
};

struct RendererState {
  Clock clock;
};

struct RenderProps {
  const RendererState &state;
  std::default_random_engine &random;

  RenderProps(const RendererState &state, std::default_random_engine &random)
      : state(state), random(random) {}
};
