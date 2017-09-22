#include "main.hpp"

#include "ParticleSpacingEffect.hpp"

constexpr const char *ParticleSpacingEffect::Name;

const char *ParticleSpacingEffect::getName() const { return Name; }
const char *ParticleSpacingEffect::getDescriptiveName() const {
  return "Particle spacing";
}
const char *ParticleSpacingEffect::getDescription() const {
  return "Adds or removes space between particles";
}

static auto EaseFunctionStrings = {
  "sine",
  "linear",
  "none",
};

void ParticleSpacingEffect::loadConfig(const json &json) {
  xSpread = json.value("xSpread", 0.f);
  ySpread = json.value("ySpread", 0.f);
  easeInTime = json.value("easeInTime", 1000.f);
  easeOutTime = json.value("easeOutTime", 1000.f);
  jsonEnumValue(json, "easeFunc", EaseFunction::Sine, EaseFunctionStrings);
}
void ParticleSpacingEffect::saveConfig(json &json) const {
  json.emplace("xSpread", xSpread);
  json.emplace("ySpread", ySpread);
  json.emplace("easeInTime", easeInTime);
  json.emplace("easeOutTime", easeOutTime);
  jsonEnumEmplace(json, "easeFunc", easeFunc, EaseFunctionStrings);
}

void ParticleSpacingEffect::randomizeConfig(std::default_random_engine &random) {
  // uniform distribution is not really correct here...
  xSpread = std::uniform_real_distribution<float>(0.5f, 2.f)(random);
  ySpread = std::uniform_real_distribution<float>(0.5f, 2.f)(random);
  easeInTime = std::uniform_real_distribution<float>(1000.f, 1500.f)(random);
  easeOutTime = std::uniform_real_distribution<float>(1000.f, 1500.f)(random);
  easeFunc = EaseFunction::Sine;
}

void ParticleSpacingEffect::registerEffect(EffectRegistrationData &data) const {
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
    vec2 offset = (initialPosition.xy * 2.f - 1.f) * ${s};
    position.xy += offset * ease;
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "s", GLSLType::Vec2,
                      [this](const RenderProps &props) {
                        return UniformValue((glm::vec2(xSpread, ySpread) - 1.f) / 2.f);
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
                        return UniformValue(glm::min(1.f, (1.f - time) / (easeInTime / getPeriod())));
                      }),
          })
          .c_str());
}

void ParticleSpacingEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
