#pragma once

#include "sound/SampleBuffer.hpp"

class SampleLibrary {
public:
  bool create(std::default_random_engine &random, Resources *resources);
  void destroy();

  const sound::SampleBuffer *getSample(const char *name) const;

  const sound::SampleBuffer *getBackgroundLoop() const;

  const sound::SampleBuffer *getRandomWhoosh() const;

private:
  std::default_random_engine *random;

  std::map<std::string, sound::SampleBuffer> samples;
};
