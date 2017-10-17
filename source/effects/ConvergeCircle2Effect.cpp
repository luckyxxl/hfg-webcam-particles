#include "main.hpp"

#include "ConvergeCircle2Effect.hpp"

constexpr const char *ConvergeCircle2Effect::Name;

const char *ConvergeCircle2Effect::getName() const { return Name; }
const char *ConvergeCircle2Effect::getDescriptiveName() const {
  return "Converge to circle 2";
}
const char *ConvergeCircle2Effect::getDescription() const {
  return "Particles are attracted towards their position on an HSV color wheel "
         "centered around the center of the screen";
}

void ConvergeCircle2Effect::loadConfig(const json &json) {
  loadEaseInOutConfig(json);
  radius = json.value("radius", .8f);
  rotationSpeed = json.value("rotationSpeed", 0.f);
}
void ConvergeCircle2Effect::saveConfig(json &json) const {
  saveEaseInOutConfig(json);
  json.emplace("radius", radius);
  json.emplace("rotationSpeed", rotationSpeed);
}

void ConvergeCircle2Effect::randomizeConfig(std::default_random_engine &random) {
  rotationSpeed = std::uniform_real_distribution<float>(-1.f, 1.f)(random);
}

void ConvergeCircle2Effect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    vec2 screenTarget = getDirectionVector(hsv[0] + ${rotation}) * vec2(${radius}) * vec2(invScreenAspectRatio, 1.);
    vec2 target = (invViewProjectionMatrix * vec4(screenTarget, 0, 1)).xy;

    vec2 d = target - initialPosition.xy;

    position.xy += mix(vec2(0.), d, ${x});
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "x", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result = getEase(props);
                        return UniformValue(result);
                      }),
              UNIFORM(data.uniforms, "rotation", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result = (props.state.clock.getTime() - timeBegin) * rotationSpeed / 1000.f;
                        return UniformValue(result);
                      }),
              UNIFORM(data.uniforms, "radius", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(radius);
                      }),
          })
          .c_str());
}

void ConvergeCircle2Effect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
