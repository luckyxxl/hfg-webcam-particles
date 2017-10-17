#include "main.hpp"

#include "ConvergePointEffect.hpp"

constexpr const char *ConvergePointEffect::Name;

const char *ConvergePointEffect::getName() const { return Name; }
const char *ConvergePointEffect::getDescriptiveName() const {
  return "Converge to point";
}
const char *ConvergePointEffect::getDescription() const {
  return "Particles are attracted towards the center of the screen";
}

void ConvergePointEffect::loadConfig(const json &json) {
}
void ConvergePointEffect::saveConfig(json &json) const {
}

void ConvergePointEffect::randomizeConfig(std::default_random_engine &random) {
}

void ConvergePointEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    vec2 screenTarget = vec2(0., 0.);
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
              UNIFORM(data.uniforms, "maxTravelTime", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(getPeriod() / 2);
                      }),
          })
          .c_str());
}

void ConvergePointEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
#if 0
  for(auto i = 0u; i < repetitions; ++i) {
    data.soundPlaylist.add(data.sampleLibrary->getRandomWhoosh(), timeBegin + i * getPeriod());
    data.soundPlaylist.add(data.sampleLibrary->getRandomWhoosh(), timeBegin + getPeriod() / 3.f + i * getPeriod());
    data.soundPlaylist.add(data.sampleLibrary->getRandomWhoosh(), timeBegin + getPeriod() / 2.f + i * getPeriod());
  }
#endif
}
