#pragma once

#include "SampleBuffer.hpp"

namespace sound {

class Renderer {
  public:
  bool create();
  void destroy();

  void update();

  bool play(const SampleBuffer *sampleBuffer);

  // don't call this!
  void audioCallback(Uint8 *stream, int len);

  private:
  SDL_AudioDeviceID audioDevice;
  SDL_AudioSpec audioSpec;

  struct Voice {
    enum class State {
      Empty,
      Playing,
    };

    std::atomic<const SampleBuffer*> sampleBuffer; // modified by host thread

    std::atomic<State> state; // modified by host thread
    std::atomic<uint32_t> cursor; // modified by host thread and audio thread
  };

  std::array<Voice, 16> voices;
};

}
