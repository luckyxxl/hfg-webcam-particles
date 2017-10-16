#include "main.hpp"

#include "ConvergePoint2Effect.hpp"

constexpr const char *ConvergePoint2Effect::Name;

const char *ConvergePoint2Effect::getName() const { return Name; }
const char *ConvergePoint2Effect::getDescriptiveName() const {
  return "Converge to point 2";
}
const char *ConvergePoint2Effect::getDescription() const {
  return "Particles are attracted towards the center of the screen";
}

void ConvergePoint2Effect::loadConfig(const json &json) {
  loadEaseInOutConfig(json);
}
void ConvergePoint2Effect::saveConfig(json &json) const {
  saveEaseInOutConfig(json);
}

void ConvergePoint2Effect::randomizeConfig(std::default_random_engine &random) {
}

void ConvergePoint2Effect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    vec2 screenTarget = vec2(0., 0.);
    vec2 target = (invViewProjectionMatrix * vec4(screenTarget, 0, 1)).xy;

    vec2 d = target - initialPosition.xy;
    float d_len = length(d);

    position.xy += mix(vec2(0.), d, ${x});
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "x", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result = getEase(props);
                        return UniformValue(result);
                      }),
          })
          .c_str());
}

void ConvergePoint2Effect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
