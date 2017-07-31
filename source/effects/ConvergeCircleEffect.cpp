#include "main.hpp"

#include "ConvergeCircleEffect.hpp"

const char *ConvergeCircleEffect::getName() const {
  return "ConvergeCircle";
}
const char *ConvergeCircleEffect::getDescriptiveName() const {
  return "Converge to circle";
}
const char *ConvergeCircleEffect::getDescription() const {
  return "Particles are attracted towards their position on an HSV color wheel centered around the center of the screen";
}

void ConvergeCircleEffect::Config::load(const json &json) {
  rotationSpeed = json.value("rotationSpeed", 0.f);
}
void ConvergeCircleEffect::Config::save(json &json) const {
  json.emplace("rotationSpeed", rotationSpeed);
}

std::unique_ptr<IEffect::IConfig> ConvergeCircleEffect::getDefaultConfig() const {
  return std::make_unique<Config>();
}
std::unique_ptr<IEffect::IConfig> ConvergeCircleEffect::getRandomConfig() const {
  return std::make_unique<Config>();
}

void ConvergeCircleEffect::writeVertexShader(const IConfig *config) const {
  
}

/*
void ConvergeCircleEffect::writeFragmentShader(const IConfig *config) const {
  
}

void ConvergeCircleEffect::scheduleSound(const IConfig *config) const {
  
}
*/
