#include "main.hpp"

#include "SampleLibrary.hpp"

bool SampleLibrary::create(std::default_random_engine &random,
                           Resources *resources) {
  this->random = &random;

  auto loadSample = [&](const std::string &name) {
    samples[name].loadFromFile(resources, ("sound/" + name + ".wav").c_str());
  };

  loadSample("DroneLoopStereo01");
  loadSample("FXStereo01");
  loadSample("FXStereo02");
  loadSample("FXStereo03");
  loadSample("FXStereo04");
  loadSample("FXStereo05");
  loadSample("sweep005");
  loadSample("up_sweep018");

  //TODO: check that all samples are loaded correctly
  return true;
}

void SampleLibrary::destroy() {
}

const sound::SampleBuffer *SampleLibrary::getSample(const char *name) const {
  return &samples.at(name);
}

const sound::SampleBuffer *SampleLibrary::getBackgroundLoop() const {
  return getSample("DroneLoopStereo01");
}

const sound::SampleBuffer *SampleLibrary::getRandomWhoosh() const {
  constexpr const char *whooshSamples[] = {
    "FXStereo01",
    "FXStereo02",
    "FXStereo03",
    "FXStereo04",
    "FXStereo05",
  };
  constexpr auto whooshSamplesSize = sizeof(whooshSamples) / sizeof(*whooshSamples);

  const auto randomIndex = std::uniform_int_distribution<>(0, whooshSamplesSize-1)(*random);
  return getSample(whooshSamples[randomIndex]);
}
