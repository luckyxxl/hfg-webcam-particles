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
  loadSample("W10");
  loadSample("W11");
  loadSample("W12");
  loadSample("W13");
  loadSample("W14");
  loadSample("W15");
  loadSample("W16");
  loadSample("W17");
  loadSample("W18");
  loadSample("W19");
  loadSample("W20");
  loadSample("intro01");
  loadSample("outro01");
  loadSample("glitch");

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
    "W10",
    "W11",
    "W12",
    "W13",
    "W14",
    "W15",
    "W16",
    "W17",
    "W18",
    "W19",
    "W20",
  };
  constexpr auto whooshSamplesSize = sizeof(whooshSamples) / sizeof(*whooshSamples);

  const auto randomIndex = std::uniform_int_distribution<>(0, whooshSamplesSize-1)(*random);
  return getSample(whooshSamples[randomIndex]);
}
