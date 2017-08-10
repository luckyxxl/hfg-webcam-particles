#include "main.hpp"

#include "SampleLibrary.hpp"

bool SampleLibrary::create(std::default_random_engine &random,
                           Resources *resources) {
  this->random = &random;

  backgroundLoop.loadFromFile(resources, "sound/DroneLoopStereo01.wav");

  whooshSamples.resize(5);
  whooshSamples[0].loadFromFile(resources, "sound/FXStereo01.wav");
  whooshSamples[1].loadFromFile(resources, "sound/FXStereo02.wav");
  whooshSamples[2].loadFromFile(resources, "sound/FXStereo03.wav");
  whooshSamples[3].loadFromFile(resources, "sound/FXStereo04.wav");
  whooshSamples[4].loadFromFile(resources, "sound/FXStereo05.wav");

  return true;
}

void SampleLibrary::destroy() {
}

const sound::SampleBuffer *SampleLibrary::getBackgroundLoop() const {
  return &backgroundLoop;
}

const sound::SampleBuffer *SampleLibrary::getRandomWhoosh() const {
  return &whooshSamples[
      std::uniform_int_distribution<>(0, whooshSamples.size() - 1)(*random)];
}
