#include "main.hpp"

#include "SampleBuffer.hpp"

namespace sound {

SampleBuffer::~SampleBuffer() {
  SDL_FreeWAV(buffer);
}

bool SampleBuffer::loadFromFile(Resources *resources, const char *filename) {
  auto rwOps = resources->openFile(filename);
  if(!rwOps) return false;

  SDL_AudioSpec desiredAudioSpec;
  desiredAudioSpec.freq = 48000;
  desiredAudioSpec.format = AUDIO_S16;
  desiredAudioSpec.channels = 2;

  bufferAudioSpec = SDL_LoadWAV_RW(rwOps, 1, &desiredAudioSpec, &buffer, &bufferLength);

  return true;
}

}
