#include "main.hpp"

#include "ConvergeCircleEffect.hpp"

const char *ConvergeCircleEffect::getName() const {
  return "ConvergeCircleEffect";
}
const char *ConvergeCircleEffect::getDescriptiveName() const {
  return "Converge to circle";
}
const char *ConvergeCircleEffect::getDescription() const {
  return "Particles are attracted towards their position on an HSV color wheel centered around the center of the screen";
}

void ConvergeCircleEffect::Config::load(const json &json) {
  rotationSpeed = json.value("rotationSpeed", 0.f);
}
void ConvergeCircleEffect::Config::save(json &json) const {
  json.emplace("rotationSpeed", rotationSpeed);
}

std::unique_ptr<IEffect::IConfig> ConvergeCircleEffect::getDefaultConfig() const {
  return std::make_unique<Config>();
}
std::unique_ptr<IEffect::IConfig> ConvergeCircleEffect::getRandomConfig() const {
  return std::make_unique<Config>();
}

void ConvergeCircleEffect::registerEffect(const EffectInstance &instance, Uniforms &uniforms, ShaderBuilder &vertexShader, ShaderBuilder &fragmentShader) const {

  const auto time = uniforms.addUniform("time", GLSLType::Float, [](const EffectInstance &instance, const RenderProps &props){
    return UniformValue(std::fmod(props.state.clock.getTime() - instance.timeBegin, instance.getPeriod()));
  });
  const auto speed = uniforms.addUniform("speed", GLSLType::Float, [](const EffectInstance &instance, const RenderProps &props){
    return UniformValue(2 * 2 / (instance.getPeriod() / 2 * instance.getPeriod() / 2));
  });
  const auto rotationSpeed = uniforms.addUniform("rotationSpeed", GLSLType::Float, [](const EffectInstance &instance, const RenderProps &props){
    const Config *config = static_cast<const Config*>(instance.config.get());
    return UniformValue(config->rotationSpeed);
  });
  const auto maxTravelTime = uniforms.addUniform("maxTravelTime", GLSLType::Float, [](const EffectInstance &instance, const RenderProps &props){
    return UniformValue(instance.getPeriod() / 2);
  });

  vertexShader.appendMainBody(TEMPLATE(R"glsl(
  {
    vec2 screenTarget = getDirectionVector(hsv[0] + ${time} * ${rotationSpeed}) * vec2(.8) * vec2(invScreenAspectRatio, 1.);
    vec2 target = (invViewProjectionMatrix * vec4(screenTarget, 0, 1)).xy;

    vec2 d = target - initialPosition.xy;
    float d_len = length(d);

    float stop_t = sqrt(2. * d_len / ${speed});

    vec2 result;

    if(${time} < stop_t) {
      float t = min(${time}, stop_t);
      result = .5 * d / d_len * ${speed} * t * t;
    } else if(${time} < ${maxTravelTime}) {
      result = d;
    } else {
      float t = ${time} - ${maxTravelTime};
      //result = mix(d, vec2(0.), 1. - (1.-t) * (1.-t));
      //result = mix(d, vec2(0.), t * t);
      result = mix(d, vec2(0.), -cos(t / ${maxTravelTime} * PI) * .5 + .5);
    }

    position.xy += result;
  }
  )glsl").compile({
    {"time", time},
    {"speed", speed},
    {"rotationSpeed", rotationSpeed},
    {"maxTravelTime", maxTravelTime},
  }).c_str());
}
