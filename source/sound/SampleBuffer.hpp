#pragma once

#include "Resources.hpp"

namespace sound {

class SampleBuffer {
  public:
  ~SampleBuffer();

  bool loadFromFile(Resources *resources, const char *filename);

  const int16_t *getBuffer() const {
    return reinterpret_cast<const int16_t*>(buffer);
  }

  uint32_t getBufferLengthSamples() const {
    return bufferLength / sizeof(uint16_t) / 2;
  }

  private:
  Uint8 *buffer = nullptr;
  Uint32 bufferLength = 0;
  SDL_AudioSpec *bufferAudioSpec = nullptr;
};

}
