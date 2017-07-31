#include "main.hpp"

#include "HueDisplaceEffect.hpp"

constexpr const char *HueDisplaceEffect::Name;

const char *HueDisplaceEffect::getName() const {
  return Name;
}
const char *HueDisplaceEffect::getDescriptiveName() const {
  return "Displace by hue";
}
const char *HueDisplaceEffect::getDescription() const {
  return "Particles move into different directions depending on their hue";
}

void HueDisplaceEffect::randomizeConfig() {
  
}

void HueDisplaceEffect::loadConfig(const json &json) {
  loadInstanceConfig(json);

  auto config = json.value("config", json::object());
  distance = config.value("distance", 0.f);
  scaleByValue = config.value("scaleByValue", 0.f);
  randomDirectionOffset = config.value("randomDirectionOffset", false);
  rotate = config.value("rotate", 0.f);
}
void HueDisplaceEffect::saveConfig(json &json) const {
  saveInstanceConfig(json);

  auto config = json::object();
  config.emplace("distance", distance);
  config.emplace("scaleByValue", scaleByValue);
  config.emplace("randomDirectionOffset", randomDirectionOffset);
  config.emplace("rotate", rotate);

  json.emplace("config", config);
}

void HueDisplaceEffect::writeVertexShader() const {
  
}

/*
void HueDisplaceEffect::writeFragmentShader() const {
  
}

void HueDisplaceEffect::scheduleSound() const {
  
}
*/
