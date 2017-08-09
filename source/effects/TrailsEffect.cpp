#include "main.hpp"

#include "TrailsEffect.hpp"

constexpr const char *TrailsEffect::Name;

const char *TrailsEffect::getName() const { return Name; }
const char *TrailsEffect::getDescriptiveName() const {
  return "Trails";
}
const char *TrailsEffect::getDescription() const {
  return "Enables an fading image echo";
}

void TrailsEffect::loadConfig(const json &json) {
  loadFadeConfig(json);
  strength = json.value("strength", .7f);
}
void TrailsEffect::saveConfig(json &json) const {
  saveFadeConfig(json);
  json.emplace("strength", strength);
}

void TrailsEffect::randomizeConfig(std::default_random_engine &random) {
  
}

void TrailsEffect::registerEffect(EffectRegistrationData &data) const {
  data.accumulationShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    accumulationEffectResult = mix(particleColor, historyColor, ${strength});
  }
  )glsl")
          .compile({
            UNIFORM(data.accumulationUniforms, "strength", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(strength);
                      }),
          })
          .c_str());
}
