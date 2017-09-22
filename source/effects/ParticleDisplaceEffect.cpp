#include "main.hpp"

#include "ParticleDisplaceEffect.hpp"

constexpr const char *ParticleDisplaceEffect::Name;

const char *ParticleDisplaceEffect::getName() const { return Name; }
const char *ParticleDisplaceEffect::getDescriptiveName() const {
  return "Displace Particles";
}
const char *ParticleDisplaceEffect::getDescription() const {
  return "Displaces all particles into a certain direction by the same distance";
}

static auto EaseFunctionStrings = {
  "sine",
  "linear",
  "none",
};

void ParticleDisplaceEffect::loadConfig(const json &json) {
  direction = json.value("direction", 0.f);
  distance = json.value("distance", 0.f);
  easeInTime = json.value("easeInTime", 1000.f);
  easeOutTime = json.value("easeOutTime", 1000.f);
  jsonEnumValue(json, "easeFunc", EaseFunction::Sine, EaseFunctionStrings);
}
void ParticleDisplaceEffect::saveConfig(json &json) const {
  json.emplace("direction", direction);
  json.emplace("distance", distance);
  json.emplace("easeInTime", easeInTime);
  json.emplace("easeOutTime", easeOutTime);
  jsonEnumEmplace(json, "easeFunc", easeFunc, EaseFunctionStrings);
}

void ParticleDisplaceEffect::randomizeConfig(std::default_random_engine &random) {
  direction = std::uniform_real_distribution<float>(0.f, 2.f * PI)(random);
  distance = std::uniform_real_distribution<float>(0.f, .25f)(random);
  easeInTime = std::uniform_real_distribution<float>(1000.f, 1500.f)(random);
  easeOutTime = std::uniform_real_distribution<float>(1000.f, 1500.f)(random);
  easeFunc = EaseFunction::Sine;
}

void ParticleDisplaceEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    float ease = 0.0;
    switch(${easeFunc}) {
      case 0: // sine
      ease = (1. - cos(PI * min(${easeInProgress}, ${easeOutProgress}))) / 2.;
      break;
      case 1: // linear
      ease = min(${easeInProgress}, ${easeOutProgress});
      break;
      case 2: // none
      ease = 1.0;
      break;
    }
    position.xy += getDirectionVector(${angle}) * ${distance} * ease;
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "angle", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(direction);
                      }),
              UNIFORM(data.uniforms, "distance", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(distance);
                      }),
              UNIFORM(data.uniforms, "easeFunc", GLSLType::Int,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<int>(easeFunc));
                      }),
              UNIFORM(data.uniforms, "easeInProgress", GLSLType::Float,
                      [this](const RenderProps &props) {
                        // starts at 0, goes down to 1
                        const auto time = glm::fract((props.state.clock.getTime() - timeBegin) / getPeriod());
                        return UniformValue(glm::min(1.f, time / (easeInTime / getPeriod())));
                      }),
              UNIFORM(data.uniforms, "easeOutProgress", GLSLType::Float,
                      [this](const RenderProps &props) {
                        // starts at 1, goes down to 0
                        const auto time = glm::fract((props.state.clock.getTime() - timeBegin) / getPeriod());
                        return UniformValue(glm::min(1.f, (1.f - time) / (easeOutTime / getPeriod())));
                      }),
          })
          .c_str());
}

void ParticleDisplaceEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
