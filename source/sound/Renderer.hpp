#pragma once

#include "SampleBuffer.hpp"

namespace sound {

class Voice {
  public:
  // play(), pause(), stop(), setLooping(bool), ...

  private:
  friend class Renderer;
  friend class std::array<Voice, 16>;

  Voice() = default;
  Voice(const Voice&) = delete;

  enum class State {
    Empty,
    Playing,
  };

  std::atomic<const SampleBuffer*> sampleBuffer; // modified by host thread

  std::atomic<State> state; // modified by host thread
  std::atomic<int32_t> cursor; // modified by host thread and audio thread
};

class Renderer {
  public:
  bool create();
  void destroy();

  void update();

  struct PlayParameters {
    double startDelay = 0.;
    PlayParameters &setStartDelay(double _startDelay) { startDelay = _startDelay; return *this; }
  };

  Voice *play(const SampleBuffer *sampleBuffer, const PlayParameters &parameters);
  Voice *play(const SampleBuffer *sampleBuffer) { return play(sampleBuffer, PlayParameters()); }

  // don't call this!
  void audioCallback(Uint8 *stream, int len);

  private:
  SDL_AudioDeviceID audioDevice;
  SDL_AudioSpec audioSpec;

  std::array<Voice, 16> voices;
};

}
