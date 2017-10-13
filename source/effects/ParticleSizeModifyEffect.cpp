#include "main.hpp"

#include "ParticleSizeModifyEffect.hpp"

constexpr const char *ParticleSizeModifyEffect::Name;

const char *ParticleSizeModifyEffect::getName() const { return Name; }
const char *ParticleSizeModifyEffect::getDescriptiveName() const {
  return "Modify particle size";
}
const char *ParticleSizeModifyEffect::getDescription() const {
  return "The particle size is modified by a constant value";
}

static auto EaseFunctionStrings = {
  "sine",
  "linear",
  "none",
};

void ParticleSizeModifyEffect::loadConfig(const json &json) {
  scaling = json.value("scaling", 1.f);
  easeInTime = json.value("easeInTime", 1000.f);
  easeOutTime = json.value("easeOutTime", 1000.f);
  easeFunc = jsonEnumValue(json, "easeFunc", EaseFunction::Sine, EaseFunctionStrings);
}
void ParticleSizeModifyEffect::saveConfig(json &json) const {
  json.emplace("scaling", scaling);
  json.emplace("easeInTime", easeInTime);
  json.emplace("easeOutTime", easeOutTime);
  jsonEnumEmplace(json, "easeFunc", easeFunc, EaseFunctionStrings);
}

void ParticleSizeModifyEffect::randomizeConfig(std::default_random_engine &random) {
}

void ParticleSizeModifyEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    float ease = 0.;
    switch(${easeFunc}) {
      case 0: // none
      ease = 1.f;
      break;
      case 1: // sine
      ease = (1. - cos(PI * min(${easeInProgress}, ${easeOutProgress}))) / 2.;
      break;
      case 2: // linear
      ease = min(${easeInProgress}, ${easeOutProgress});
      break;
    }

    // mix(1., ${scaling}, ease)
    pointSize *= 1. - (1. - ${scaling}) * ease;
  }
  )glsl")
          .compile({
            UNIFORM(data.uniforms, "scaling", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(scaling);
                      }),
            UNIFORM(data.uniforms, "easeInProgress", GLSLType::Float,
                      [this](const RenderProps &props) {
                        const auto time = glm::fract((props.state.clock.getTime() - timeBegin) / getPeriod());
                        const auto easeInTime = glm::min(this->easeInTime, getPeriod() / 2.f);
                        return UniformValue(glm::min(1.f, time / (easeInTime / getPeriod())));
                      }),
            UNIFORM(data.uniforms, "easeOutProgress", GLSLType::Float,
                      [this](const RenderProps &props) {
                        const auto time = glm::fract((props.state.clock.getTime() - timeBegin) / getPeriod());
                        const auto easeInTime = glm::min(this->easeInTime, getPeriod() / 2.f);
                        const auto easeOutTime = glm::min(this->easeOutTime, getPeriod() - easeInTime);
                        return UniformValue(glm::min(1.f, (1.f - time) / (easeOutTime / getPeriod())));
                      }),
            UNIFORM(data.uniforms, "easeFunc", GLSLType::Int,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<int>(easeFunc));
                      }),
          })
          .c_str());
}

void ParticleSizeModifyEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
