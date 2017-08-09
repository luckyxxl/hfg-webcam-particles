#include "main.hpp"

#include "ConvergeCircleEffect.hpp"

constexpr const char *ConvergeCircleEffect::Name;

const char *ConvergeCircleEffect::getName() const { return Name; }
const char *ConvergeCircleEffect::getDescriptiveName() const {
  return "Converge to circle";
}
const char *ConvergeCircleEffect::getDescription() const {
  return "Particles are attracted towards their position on an HSV color wheel "
         "centered around the center of the screen";
}

void ConvergeCircleEffect::loadConfig(const json &json) {
  rotationSpeed = json.value("rotationSpeed", 0.f);
}
void ConvergeCircleEffect::saveConfig(json &json) const {
  json.emplace("rotationSpeed", rotationSpeed);
}

void ConvergeCircleEffect::randomizeConfig(std::default_random_engine &random) {
  rotationSpeed = std::uniform_real_distribution<float>()(random);
}

void ConvergeCircleEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
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
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "time", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            std::fmod(props.state.clock.getTime() - timeBegin,
                                      getPeriod()));
                      }),
              UNIFORM(data.uniforms, "speed", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            2 * 2 / (getPeriod() / 2 * getPeriod() / 2));
                      }),
              UNIFORM(data.uniforms, "rotationSpeed", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(this->rotationSpeed / 1000.f);
                      }),
              UNIFORM(data.uniforms, "maxTravelTime", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(getPeriod() / 2);
                      }),
          })
          .c_str());
}
