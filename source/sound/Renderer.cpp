#include "main.hpp"

#include "Renderer.hpp"
#include "SampleBuffer.hpp"

namespace sound {

static void sdlAudioCallback(void *userdata, Uint8 *stream, int len) {
  static_cast<Renderer*>(userdata)->audioCallback(stream, len);
}

bool Renderer::create() {
  {
    SDL_AudioSpec desiredAudioSpec;
    desiredAudioSpec.freq = 48000;
    desiredAudioSpec.format = AUDIO_F32SYS;
    desiredAudioSpec.channels = 2;
    desiredAudioSpec.samples = 512;
    desiredAudioSpec.callback = sdlAudioCallback;
    desiredAudioSpec.userdata = this;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredAudioSpec, &audioSpec, 0);
    if(!audioDevice) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open audio device", SDL_GetError(), NULL);
      return false;
    }
  }

  assert(std::atomic_is_lock_free(&voices[0].sampleBuffer));
  assert(std::atomic_is_lock_free(&voices[0].state));
  assert(std::atomic_is_lock_free(&voices[0].cursor));

  for(auto &voice : voices) {
    voice.state = Voice::State::Empty;
  }

  SDL_PauseAudioDevice(audioDevice, 0);

  return true;
}

void Renderer::destroy() {
  if(audioDevice) SDL_CloseAudioDevice(audioDevice);
}

void Renderer::update() {
  for(auto &voice : voices) {
    if(voice.state.load() == Voice::State::Empty) continue;

    assert(voice.state == Voice::State::Playing);

    const auto sampleBufferValue = voice.sampleBuffer.load(std::memory_order_relaxed);
    const int32_t cursorValue = voice.cursor.load(std::memory_order_relaxed);

    const int32_t sampleBufferLengthSamples = sampleBufferValue->getBufferLengthSamples();

    if(cursorValue == sampleBufferLengthSamples) {
      voice.state.store(Voice::State::Empty);
    }
  }
}

Voice *Renderer::play(const SampleBuffer *sampleBuffer, const PlayParameters &parameters) {
  for(auto &voice : voices) {
    if(voice.state.load() != Voice::State::Empty) continue;

    voice.sampleBuffer.store(sampleBuffer, std::memory_order_relaxed);
    voice.cursor.store(-parameters.startDelay * audioSpec.freq, std::memory_order_relaxed);
    voice.looping.store(parameters.looping, std::memory_order_relaxed);

    voice.state.store(Voice::State::Playing, std::memory_order_release);

    return &voice;
  }

  std::cout << "out of voices\n";

  return nullptr;
}

void Renderer::audioCallback(Uint8 *stream, int len) {
  memset(stream, 0, len);

  float *const buffer = reinterpret_cast<float*>(stream);
  const auto bufferLengthSamples = int32_t(len / sizeof(float) / 2); // two channels per sample

  for(auto &voice : voices) {
    if(voice.state.load() == Voice::State::Empty) continue;

    auto cursorValue = voice.cursor.load(std::memory_order_relaxed);

    const auto sampleBufferValue = voice.sampleBuffer.load(std::memory_order_relaxed);

    const auto sampleBuffer = sampleBufferValue->getBuffer();
    const auto sampleBufferLengthSamples = int32_t(sampleBufferValue->getBufferLengthSamples());

    const auto loopingValue = voice.looping.load(std::memory_order_relaxed);

    // start delay
    if(cursorValue + bufferLengthSamples <= 0) {
      voice.cursor.store(cursorValue + bufferLengthSamples);
      continue;
    }

    // finished
    if(cursorValue == sampleBufferLengthSamples) {
      continue;
    }

    auto destinationOffset = cursorValue < 0 ? -cursorValue : 0;

    loop: {
      const auto copySamplesCount = std::min(bufferLengthSamples - destinationOffset, sampleBufferLengthSamples - std::max(cursorValue, 0));
      assert(copySamplesCount > 0);

      // please don't generalize/combine the cases, so that there's a better chance for optimization
      switch(sampleBufferValue->getChannels()) {
        case 1:
        {
          const auto copySrc = sampleBuffer + 1 * std::max(cursorValue, 0);
          const auto copyDest = buffer + 2 * destinationOffset;

          for(auto i=0; i<copySamplesCount; ++i) {
            copyDest[2*i+0] += copySrc[i];
            copyDest[2*i+1] += copySrc[i];
          }
        }
        break;

        case 2:
        {
          const auto copySrc = sampleBuffer + 2 * std::max(cursorValue, 0);
          const auto copyDest = buffer + 2 * destinationOffset;

          for(auto i=0; i<copySamplesCount; ++i) {
            copyDest[2*i+0] += copySrc[2*i+0];
            copyDest[2*i+1] += copySrc[2*i+1];
          }
        }
        break;
      }

      cursorValue += copySamplesCount;

      if(loopingValue && cursorValue == sampleBufferLengthSamples) {
        cursorValue = 0;
        destinationOffset += copySamplesCount;
        goto loop;
      }
    }

    voice.cursor.store(cursorValue);
  }
}

}
