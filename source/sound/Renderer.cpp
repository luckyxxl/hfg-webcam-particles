#include "main.hpp"

#include "Renderer.hpp"
#include "SampleBuffer.hpp"

namespace sound {

bool Renderer::create() {
  {
    SDL_AudioSpec desiredAudioSpec;
    desiredAudioSpec.freq = 48000;
    desiredAudioSpec.format = AUDIO_F32SYS;
    desiredAudioSpec.channels = 2;
    desiredAudioSpec.samples = 2048;
    desiredAudioSpec.callback = sdlAudioCallback;
    desiredAudioSpec.userdata = this;

    audioDevice =
        SDL_OpenAudioDevice(nullptr, 0, &desiredAudioSpec, &audioSpec, 0);
    if (!audioDevice) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                               "Could not open audio device", SDL_GetError(),
                               NULL);
      return false;
    }
  }

  std::cout << "Audio:\n"
      << "\tsamplerate: " << audioSpec.freq << "\n"
      << "\tchannels: " << (int)audioSpec.channels << "\n"
      << "\tsamples: " << audioSpec.samples << "\n";

  assert(std::atomic_is_lock_free(&voices[0].sampleBuffer));
  assert(std::atomic_is_lock_free(&voices[0].state));
  assert(std::atomic_is_lock_free(&voices[0].cursor));

  for (auto &voice : voices) {
    voice.state = Voice::State::Empty;
  }

  glitchBuffer.resize(audioSpec.freq * 2); // 1s stereo

  SDL_PauseAudioDevice(audioDevice, 0);

  return true;
}

void Renderer::destroy() {
  if (audioDevice)
    SDL_CloseAudioDevice(audioDevice);
}

void Renderer::killAllVoices() {
  SDL_LockAudioDevice(audioDevice);
  for(auto &voice : voices) {
    voice.state.store(Voice::State::Empty);
  }
  SDL_UnlockAudioDevice(audioDevice);
}

void Renderer::update() {
  auto activeVoicesCount = 0u;

  for (auto &voice : voices) {
    if (voice.state.load() == Voice::State::Empty)
      continue;

    assert(voice.state == Voice::State::Playing);
    activeVoicesCount++;

    const auto sampleBufferValue =
        voice.sampleBuffer.load(std::memory_order_relaxed);
    const int32_t cursorValue = voice.cursor.load(std::memory_order_relaxed);

    const int32_t sampleBufferLengthSamples =
        sampleBufferValue->getBufferLengthSamples();

    if (cursorValue == sampleBufferLengthSamples) {
      voice.state.store(Voice::State::Empty);
    }
  }

#if 0
  std::cout << "active voices: " << activeVoicesCount << "\n";
#endif
}

Voice *Renderer::play(const SampleBuffer *sampleBuffer,
                      const PlayParameters &parameters) {
  assert(sampleBuffer);

  for (auto &voice : voices) {
    if (voice.state.load() != Voice::State::Empty)
      continue;

    voice.sampleBuffer.store(sampleBuffer, std::memory_order_relaxed);
    voice.cursor.store(-parameters.startDelay * audioSpec.freq / 1000.,
                       std::memory_order_relaxed);
    voice.looping.store(parameters.looping, std::memory_order_relaxed);

    voice.state.store(Voice::State::Playing, std::memory_order_release);

    return &voice;
  }

  std::cout << "out of voices\n";

  return nullptr;
}

void Renderer::glitch(double startDelay, double length, double bufferLength) {
  if(glitchActive.load(std::memory_order_acquire)) return;

  glitchSample = -startDelay * audioSpec.freq / 1000.;
  glitchLengthSamples = length * audioSpec.freq / 1000.;
  glitchBufferLengthSamples = std::min<int32_t>(bufferLength  * audioSpec.freq / 1000., glitchBuffer.size() / 2u);

  // shouldn't be a problem, but you won't hear any difference if this condition is false
  assert(glitchBufferLengthSamples < glitchLengthSamples);

  glitchActive.store(true, std::memory_order_release);
}

void Renderer::sdlAudioCallback(void *userdata, Uint8 *stream, int len) {
  static_cast<Renderer *>(userdata)->audioCallback(stream, len);
}

void Renderer::audioCallback(Uint8 *stream, int len) {
  memset(stream, 0, len);

  float *const buffer = reinterpret_cast<float *>(stream);
  const auto bufferLengthSamples =
      int32_t(len / sizeof(float) / 2); // two channels per sample

  for (auto &voice : voices) {
    if (voice.state.load() == Voice::State::Empty)
      continue;

    auto cursorValue = voice.cursor.load(std::memory_order_relaxed);

    const auto sampleBufferValue =
        voice.sampleBuffer.load(std::memory_order_relaxed);

    const auto sampleBuffer = sampleBufferValue->getBuffer();
    const auto sampleBufferLengthSamples =
        int32_t(sampleBufferValue->getBufferLengthSamples());

    const auto loopingValue = voice.looping.load(std::memory_order_relaxed);

    // start delay
    if (cursorValue + bufferLengthSamples <= 0) {
      voice.cursor.store(cursorValue + bufferLengthSamples);
      continue;
    }

    // finished
    if (cursorValue == sampleBufferLengthSamples) {
      continue;
    }

    auto destinationOffset = cursorValue < 0 ? -cursorValue : 0;

  loop : {
    const auto copySamplesCount =
        std::min(bufferLengthSamples - destinationOffset,
                 sampleBufferLengthSamples - std::max(cursorValue, 0));
    assert(copySamplesCount > 0);

    // please don't generalize/combine the cases, so that there's a better
    // chance for optimization
    switch (sampleBufferValue->getChannels()) {
    case 1: {
      const auto copySrc = sampleBuffer + 1 * std::max(cursorValue, 0);
      const auto copyDest = buffer + 2 * destinationOffset;

      for (auto i = 0; i < copySamplesCount; ++i) {
        copyDest[2 * i + 0] += copySrc[i];
        copyDest[2 * i + 1] += copySrc[i];
      }
    } break;

    case 2: {
      const auto copySrc = sampleBuffer + 2 * std::max(cursorValue, 0);
      const auto copyDest = buffer + 2 * destinationOffset;

      for (auto i = 0; i < copySamplesCount; ++i) {
        copyDest[2 * i + 0] += copySrc[2 * i + 0];
        copyDest[2 * i + 1] += copySrc[2 * i + 1];
      }
    } break;
    }

    cursorValue += copySamplesCount;

    if (loopingValue && cursorValue == sampleBufferLengthSamples) {
      cursorValue = 0;
      destinationOffset += copySamplesCount;
      if(destinationOffset != bufferLengthSamples) {
        goto loop;
      }
    }
  }

    voice.cursor.store(cursorValue);
  }

  if(glitchActive.load(std::memory_order_acquire)) {
    int32_t bufferSample = 0;

    if(glitchSample < 0) {
      auto o = std::min(-glitchSample, bufferLengthSamples);
      glitchSample += o;
      bufferSample += o;
    }

    if(glitchSample >= 0) {
      for(; glitchSample < glitchBufferLengthSamples && bufferSample < bufferLengthSamples; ++glitchSample, ++bufferSample) {
        glitchBuffer[2*glitchSample+0] = buffer[2*bufferSample+0];
        glitchBuffer[2*glitchSample+1] = buffer[2*bufferSample+1];
      }

      auto gi = glitchSample % glitchBufferLengthSamples;
      for(; glitchSample < glitchLengthSamples && bufferSample < bufferLengthSamples; ++glitchSample, ++bufferSample) {
        buffer[2*bufferSample+0] = glitchBuffer[2*gi+0];
        buffer[2*bufferSample+1] = glitchBuffer[2*gi+1];
        if(++gi == glitchBufferLengthSamples) gi = 0;
      }

      if(glitchSample == glitchLengthSamples) {
        glitchActive.store(false, std::memory_order_release);
      }
    }
  }

  for(auto i=0; i<bufferLengthSamples*2; ++i) {
    buffer[i] *= .5f;
    if(buffer[i] > 1.f) std::cout << "clip\n";
  }

  // disable sound (for listening to smooth jazz while coding :) )
#if 0
  memset(stream, 0, len);
#endif
}

} // namespace sound
