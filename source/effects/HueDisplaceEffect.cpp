#include "main.hpp"

#include "HueDisplaceEffect.hpp"

const char *HueDisplaceEffect::getName() const {
  return "HueDisplaceEffect";
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

void HueDisplaceEffect::registerEffect(const EffectInstance &instance, Uniforms &uniforms, ShaderBuilder &vertexShader, ShaderBuilder &fragmentShader) const {
  const Config *config = static_cast<const Config*>(instance.config.get());
  if(config->distance != 0.f) {
    const auto distance = uniforms.addUniform("distance", GLSLType::Float, [](const EffectInstance &instance) {
      const Config *config = static_cast<const Config*>(instance.config.get());
      return UniformValue(config->distance);
    });
    const auto time = uniforms.addUniform("time", GLSLType::Float, [](const EffectInstance &instance) {
      //const Config *config = static_cast<const Config*>(instance.config.get());
      return UniformValue((/*props.clock.getTime() -*/ instance.timeBegin) / instance.getPeriod() * 2 * PI);
    });
    const auto directionOffset = uniforms.addUniform("directionOffset", GLSLType::Float, [](const EffectInstance &instance) {
      const Config *config = static_cast<const Config*>(instance.config.get());
      auto result = config->rotate * (/*props.clock.getTime() -*/ instance.timeBegin) / instance.getPeriod() * 2 * PI;
      if(config->randomDirectionOffset) {
        if(std::isnan(config->randomDirectionOffsetValue)) {
          const_cast<Config*>(config)->randomDirectionOffsetValue = /* random() **/ 2 * PI;
        }
        result += config->randomDirectionOffsetValue;
      }
      return UniformValue(result);
    });
    const auto scaleByVal = uniforms.addUniform("scaleByVal", GLSLType::Float, [](const EffectInstance &instance) {
      const Config *config = static_cast<const Config*>(instance.config.get());
      return UniformValue(config->scaleByValue);
    });

    vertexShader.appendMainBody(TEMPLATE(R"glsl(
    {
      float angle = hsv[0] + ${directionOffset};
      float offset = (-cos(${time}) + 1.) / 2.;
      position.xy += offset * getDirectionVector(angle) * ${distance} * (1. - ${scaleByVal} * (1. - hsv[2]));
    }
    )glsl").compile({
      {"distance", distance},
      {"time", time},
      {"directionOffset", directionOffset},
      {"scaleByVal", scaleByVal},
    }).c_str());
  }
}
