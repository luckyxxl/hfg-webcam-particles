#include "main.hpp"

#include "ReduceParticleCountEffect.hpp"

constexpr const char *ReduceParticleCountEffect::Name;

const char *ReduceParticleCountEffect::getName() const { return Name; }
const char *ReduceParticleCountEffect::getDescriptiveName() const {
  return "Reduce particle count";
}
const char *ReduceParticleCountEffect::getDescription() const {
  return "Reduces the amount of visible particles";
}

static auto EaseFunctionStrings = {
  "sine",
  "linear",
  "none",
};

void ReduceParticleCountEffect::loadConfig(const json &json) {
  amount = json.value("amount", 2u);
  easeInTime = json.value("easeInTime", 1000.f);
  easeOutTime = json.value("easeOutTime", 1000.f);
  easeFunc = jsonEnumValue(json, "easeFunc", EaseFunction::Sine, EaseFunctionStrings);
}
void ReduceParticleCountEffect::saveConfig(json &json) const {
  json.emplace("amount", amount);
  json.emplace("easeInTime", easeInTime);
  json.emplace("easeOutTime", easeOutTime);
  jsonEnumEmplace(json, "easeFunc", easeFunc, EaseFunctionStrings);
}

void ReduceParticleCountEffect::randomizeConfig(std::default_random_engine &random) {
}

void ReduceParticleCountEffect::registerEffect(EffectRegistrationData &data) const {
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

    if((gl_VertexID - pixelPosition.y * 8) % ${amount} != 0) {
      visibility *= 1. - ease;
      pointSize *= 1. - ease;
    }
  }
  )glsl")
          .compile({
            UNIFORM(data.uniforms, "amount", GLSLType::Int,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<int>(amount));
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

void ReduceParticleCountEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
