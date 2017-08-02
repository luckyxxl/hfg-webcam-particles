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

void HueDisplaceEffect::loadConfig(const json &json) {
  distance = json.value("distance", 0.f);
  scaleByValue = json.value("scaleByValue", 0.f);
  randomDirectionOffset = json.value("randomDirectionOffset", false);
  rotate = json.value("rotate", 0.f);
}
void HueDisplaceEffect::saveConfig(json &json) const {
  json.emplace("distance", distance);
  json.emplace("scaleByValue", scaleByValue);
  json.emplace("randomDirectionOffset", randomDirectionOffset);
  json.emplace("rotate", rotate);
}

void HueDisplaceEffect::randomizeConfig() {
  
}

void HueDisplaceEffect::registerEffect(Uniforms &uniforms, ShaderBuilder &vertexShader, ShaderBuilder &fragmentShader) const {
  if(distance != 0.f) {
    const auto distance = uniforms.addUniform("distance", GLSLType::Float, [this](const RenderProps &props) {
      //TODO: remove this->
      return UniformValue(this->distance);
    });
    const auto time = uniforms.addUniform("time", GLSLType::Float, [this](const RenderProps &props) {
      return UniformValue((props.state.clock.getTime() - timeBegin) / getPeriod() * 2 * PI);
    });
    const auto directionOffset = uniforms.addUniform("directionOffset", GLSLType::Float, [this](const RenderProps &props) {
      auto result = rotate * (props.state.clock.getTime() - timeBegin) / getPeriod() * 2 * PI;
      if(randomDirectionOffset) {
        if(std::isnan(randomDirectionOffsetValue)) {
          std::uniform_real_distribution<float> dist;
          const_cast<HueDisplaceEffect*>(this)->randomDirectionOffsetValue = dist(props.random) * 2 * PI;
        }
        result += randomDirectionOffsetValue;
      }
      return UniformValue(result);
    });
    const auto scaleByVal = uniforms.addUniform("scaleByVal", GLSLType::Float, [this](const RenderProps &props) {
      //TODO: remove this->
      return UniformValue(this->scaleByValue);
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
