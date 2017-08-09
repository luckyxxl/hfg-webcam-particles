#include "main.hpp"

#include "HueDisplaceEffect.hpp"

constexpr const char *HueDisplaceEffect::Name;

const char *HueDisplaceEffect::getName() const { return Name; }
const char *HueDisplaceEffect::getDescriptiveName() const {
  return "Displace by hue";
}
const char *HueDisplaceEffect::getDescription() const {
  return "Particles move into different directions depending on their hue";
}

void HueDisplaceEffect::loadConfig(const json &json) {
  distance = json.value("distance", 0.f);
  scaleByValue = json.value("scaleByValue", 0.f);
  scaleByForegroundMask = json.value("scaleByForegroundMask", 0.f);
  randomDirectionOffset = json.value("randomDirectionOffset", false);
  rotate = json.value("rotate", 0.f);
}
void HueDisplaceEffect::saveConfig(json &json) const {
  json.emplace("distance", distance);
  json.emplace("scaleByValue", scaleByValue);
  json.emplace("scaleByForegroundMask", scaleByForegroundMask);
  json.emplace("randomDirectionOffset", randomDirectionOffset);
  json.emplace("rotate", rotate);
}

void HueDisplaceEffect::randomizeConfig(std::default_random_engine &random) {
  distance = std::uniform_real_distribution<float>()(random);
  scaleByValue = std::uniform_real_distribution<float>()(random);
  scaleByForegroundMask = 0.f;
  randomDirectionOffset = std::bernoulli_distribution()(random);
  rotate = std::uniform_real_distribution<float>()(random);
}

void HueDisplaceEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    float angle = hsv[0] + ${directionOffset};
    float offset = (-cos(${time}) + 1.) / 2.;
    position.xy += offset * getDirectionVector(angle) * ${distance} * (1. - ${scaleByVal} * (1. - hsv[2])) * (1. - ${scaleByFg} * (1. - foregroundMask));
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "distance", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(distance);
                      }),
              UNIFORM(data.uniforms, "time", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            (props.state.clock.getTime() - timeBegin) /
                            getPeriod() * 2 * PI);
                      }),
              UNIFORM(data.uniforms, "directionOffset", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result =
                            rotate *
                            (props.state.clock.getTime() - timeBegin) /
                            getPeriod() * 2 * PI;
                        if (randomDirectionOffset) {
                          if (std::isnan(randomDirectionOffsetValue)) {
                            std::uniform_real_distribution<float> dist;
                            const_cast<HueDisplaceEffect *>(this)
                                ->randomDirectionOffsetValue =
                                dist(props.random) * 2 * PI;
                          }
                          result += randomDirectionOffsetValue;
                        }
                        return UniformValue(result);
                      }),
              UNIFORM(data.uniforms, "scaleByVal", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(scaleByValue);
                      }),
              UNIFORM(data.uniforms, "scaleByFg", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(scaleByForegroundMask);
                      }),
          })
          .c_str());
}
