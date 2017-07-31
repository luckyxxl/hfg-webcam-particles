#include "main.hpp"

#include "HueDisplaceEffect.hpp"

const char *HueDisplaceEffect::getName() const {
  return "HueDisplace";
}
const char *HueDisplaceEffect::getDescriptiveName() const {
  return "Displace by hue";
}
const char *HueDisplaceEffect::getDescription() const {
  return "Particles move into different directions depending on their hue";
}

void HueDisplaceEffect::Config::load(const json &json) {
  distance = json.value("distance", 0.f);
  scaleByValue = json.value("scaleByValue", 0.f);
  randomDirectionOffset = json.value("randomDirectionOffset", false);
  rotate = json.value("rotate", 0.f);
}
void HueDisplaceEffect::Config::save(json &json) const {
  json.emplace("distance", distance);
  json.emplace("scaleByValue", scaleByValue);
  json.emplace("randomDirectionOffset", randomDirectionOffset);
  json.emplace("rotate", rotate);
}

std::unique_ptr<IEffect::IConfig> HueDisplaceEffect::getDefaultConfig() const {
  return std::make_unique<Config>();
}
std::unique_ptr<IEffect::IConfig> HueDisplaceEffect::getRandomConfig() const {
  return std::make_unique<Config>();
}

void HueDisplaceEffect::writeVertexShader(const IConfig *config) const {
  
}

/*
void HueDisplaceEffect::writeFragmentShader(const IConfig *config) const {
  
}

void HueDisplaceEffect::scheduleSound(const IConfig *config) const {
  
}
*/
