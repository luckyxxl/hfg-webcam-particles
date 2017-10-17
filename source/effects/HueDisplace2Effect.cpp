#include "main.hpp"

#include "HueDisplace2Effect.hpp"

constexpr const char *HueDisplace2Effect::Name;

const char *HueDisplace2Effect::getName() const { return Name; }
const char *HueDisplace2Effect::getDescriptiveName() const {
  return "Displace by hue 2";
}
const char *HueDisplace2Effect::getDescription() const {
  return "Particles move into different directions depending on their hue";
}

void HueDisplace2Effect::loadConfig(const json &json) {
  loadEaseInOutConfig(json);
  distance = json.value("distance", 0.f);
  scaleByValue = json.value("scaleByValue", 0.f);
  directionOffset = json.value("directionOffset", 0.f);
  rotate = json.value("rotate", 0.f);
}
void HueDisplace2Effect::saveConfig(json &json) const {
  saveEaseInOutConfig(json);
  json.emplace("distance", distance);
  json.emplace("scaleByValue", scaleByValue);
  json.emplace("directionOffset", directionOffset);
  json.emplace("rotate", rotate);
}

void HueDisplace2Effect::randomizeConfig(std::default_random_engine &random) {
  easeInTime = std::min(std::uniform_real_distribution<float>(1000.f, 5000.f)(random), getPeriod() / 2.f);
  easeInFunction = IEaseInOutEffect::EaseFunction::SineInOut;
  easeOutTime = std::min(std::uniform_real_distribution<float>(1000.f, 5000.f)(random), getPeriod() / 2.f);
  easeOutFunction = IEaseInOutEffect::EaseFunction::SineInOut;
  distance = std::uniform_real_distribution<float>(0.f, .5f)(random);
  scaleByValue = std::uniform_real_distribution<float>()(random);
  directionOffset = std::uniform_real_distribution<float>(0.f, 2.f*PI)(random);
  rotate = std::uniform_real_distribution<float>(-1.f, 1.f)(random);
}

void HueDisplace2Effect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    float angle = hsv[0] + ${directionOffset};
    position.xy += ${offset} * getDirectionVector(angle) * (1. - ${scaleByVal} * (1. - hsv[2]));
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "offset", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result = getEase(props) * distance;
                        return UniformValue(result);
                      }),
              UNIFORM(data.uniforms, "scaleByVal", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(scaleByValue);
                      }),
              UNIFORM(data.uniforms, "directionOffset", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result = directionOffset +
                            rotate *
                            (props.state.clock.getTime() - timeBegin) /
                            getPeriod() * 2 * PI;
                        return UniformValue(result);
                      }),
          })
          .c_str());
}

void HueDisplace2Effect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
