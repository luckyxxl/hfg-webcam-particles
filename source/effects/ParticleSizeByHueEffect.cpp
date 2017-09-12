#include "main.hpp"

#include "ParticleSizeByHueEffect.hpp"

constexpr const char *ParticleSizeByHueEffect::Name;

const char *ParticleSizeByHueEffect::getName() const { return Name; }
const char *ParticleSizeByHueEffect::getDescriptiveName() const {
  return "Particle size by hue";
}
const char *ParticleSizeByHueEffect::getDescription() const {
  return "Particles will have different sizes depending on their color";
}

static auto EaseFunctionStrings = {
  "sine",
  "linear",
  "none",
};

void ParticleSizeByHueEffect::loadConfig(const json &json) {
  scaling = json.value("scaling", 1.f);
  hueRotation = json.value("hueRotation", 0.f);
  easeInTime = json.value("easeInTime", 1000.f);
  easeOutTime = json.value("easeOutTime", 1000.f);
  easeFunc = jsonEnumValue(json, "easeFunc", EaseFunction::Sine, EaseFunctionStrings);
}
void ParticleSizeByHueEffect::saveConfig(json &json) const {
  json.emplace("scaling", scaling);
  json.emplace("hueRotation", hueRotation);
  json.emplace("easeInTime", easeInTime);
  json.emplace("easeOutTime", easeOutTime);
  jsonEnumEmplace(json, "easeFunc", easeFunc, EaseFunctionStrings);
}

void ParticleSizeByHueEffect::randomizeConfig(std::default_random_engine &random) {
  scaling = std::uniform_real_distribution<float>(0.f, 3.f)(random);
  hueRotation = std::uniform_real_distribution<float>(0.f, 2.f * PI)(random);
  easeInTime = 1000.f;
  easeOutTime = 1000.f;
  easeFunc = static_cast<EaseFunction>(std::uniform_int_distribution<int>(0, 2)(random));
}

void ParticleSizeByHueEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    // Generate a number between 0 and 1 depending on position on hue wheel
    float huePosition = fract((hsv[0] + ${hueRotation}) / (2. * PI));
    // A scaling value of 0.5 means a decrease by 50%
    float increase = (float(${scaling}) - 1.) * huePosition;
    float sizeDiff = increase * pointSize;

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

    pointSize += sizeDiff * ease;
  }
  )glsl")
          .compile({
            UNIFORM(data.uniforms, "scaling", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(scaling);
                      }),
            UNIFORM(data.uniforms, "hueRotation", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(hueRotation);
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

void ParticleSizeByHueEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
