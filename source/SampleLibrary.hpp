#pragma once

#include "sound/SampleBuffer.hpp"

class SampleLibrary {
public:
  bool create(std::default_random_engine &random, Resources *resources);
  void destroy();

  const sound::SampleBuffer *getBackgroundLoop() const;

  const sound::SampleBuffer *getRandomWhoosh() const;

private:
  std::default_random_engine *random;

  sound::SampleBuffer backgroundLoop;
  std::vector<sound::SampleBuffer> whooshSamples;
};
