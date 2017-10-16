#pragma once

namespace sound {
class SampleBuffer;

class Voice {
public:
  // play(), pause(), stop(), setLooping(bool), ...

private:
  friend class Renderer;
  friend class std::array<Voice, 16>;

  Voice() = default;
  Voice(const Voice &) = delete;

  enum class State {
    Empty,
    Playing,
  };

  // TODO: rethink this lock-free stuff

  std::atomic<const SampleBuffer *> sampleBuffer; // modified by host thread

  std::atomic<State> state;    // modified by host thread
  std::atomic<int32_t> cursor; // modified by host thread and audio thread

  std::atomic<bool> looping; // modified by host thread
};

class Renderer {
public:
  bool create();
  void destroy();

  // Blocks until all voices are killed so that all SampleBuffers can be safely
  // deleted.
  void killAllVoices();

  void update();

  struct PlayParameters {
    double startDelay = 0.;
    PlayParameters &setStartDelay(double _startDelay) {
      startDelay = _startDelay;
      return *this;
    }

    bool looping = false;
    PlayParameters &setLooping(bool _looping) {
      looping = _looping;
      return *this;
    }
  };

  Voice *play(const SampleBuffer *sampleBuffer,
              const PlayParameters &parameters);
  Voice *play(const SampleBuffer *sampleBuffer) {
    return play(sampleBuffer, PlayParameters());
  }

private:
  SDL_AudioDeviceID audioDevice;
  SDL_AudioSpec audioSpec;

  std::array<Voice, 16> voices;

  void audioCallback(Uint8 *stream, int len);
  static void sdlAudioCallback(void *userdata, Uint8 *stream, int len);
};

} // namespace sound
