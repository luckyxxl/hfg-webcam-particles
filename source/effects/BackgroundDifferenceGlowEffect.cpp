#include "main.hpp"

#include "BackgroundDifferenceGlowEffect.hpp"

constexpr const char *BackgroundDifferenceGlowEffect::Name;

const char *BackgroundDifferenceGlowEffect::getName() const { return Name; }
const char *BackgroundDifferenceGlowEffect::getDescriptiveName() const {
  return "Backgroudn difference glow";
}
const char *BackgroundDifferenceGlowEffect::getDescription() const {
  return "Make all particles dark and light only those that are different from the background";
}

void BackgroundDifferenceGlowEffect::loadConfig(const json &json) {
}
void BackgroundDifferenceGlowEffect::saveConfig(json &json) const {
}

void BackgroundDifferenceGlowEffect::randomizeConfig(std::default_random_engine &random) {
}

void BackgroundDifferenceGlowEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    float x = foregroundMask;
    rgb = vec3(1.0) * x;
  }
  )glsl")
          .compile({
          })
          .c_str());
}

void BackgroundDifferenceGlowEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
