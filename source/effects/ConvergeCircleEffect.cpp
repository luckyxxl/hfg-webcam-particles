#include "main.hpp"

#include "ConvergeCircleEffect.hpp"

constexpr const char *ConvergeCircleEffect::Name;

const char *ConvergeCircleEffect::getName() const {
  return Name;
}
const char *ConvergeCircleEffect::getDescriptiveName() const {
  return "Converge to circle";
}
const char *ConvergeCircleEffect::getDescription() const {
  return "Particles are attracted towards their position on an HSV color wheel centered around the center of the screen";
}

void ConvergeCircleEffect::randomizeConfig() {
  
}

void ConvergeCircleEffect::loadConfig(const json &json) {
  loadInstanceConfig(json);

  auto config = json.value("config", json::object());
  rotationSpeed = config.value("rotationSpeed", 0.f);
}
void ConvergeCircleEffect::saveConfig(json &json) const {
  saveInstanceConfig(json);

  auto config = json::object();
  config.emplace("rotationSpeed", rotationSpeed);

  json.emplace("config", config);
}

void ConvergeCircleEffect::writeVertexShader() const {
  
}

/*
void ConvergeCircleEffect::writeFragmentShader() const {
  
}

void ConvergeCircleEffect::scheduleSound() const {
  
}
*/
