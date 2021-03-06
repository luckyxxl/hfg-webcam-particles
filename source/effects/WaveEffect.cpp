#include "main.hpp"

#include "WaveEffect.hpp"

constexpr const char *WaveEffect::Name;

const char *WaveEffect::getName() const { return Name; }
const char *WaveEffect::getDescriptiveName() const {
  return "Wave";
}
const char *WaveEffect::getDescription() const {
  return "A wave passes through the particles from left to right over the "
         "screen";
}

void WaveEffect::loadConfig(const json &json) {
  multiplier = json.value("multiplier", 1.f);
  amplitude = json.value("amplitude", .05f);
}
void WaveEffect::saveConfig(json &json) const {
  json.emplace("multiplier", multiplier);
  json.emplace("amplitude", amplitude);
}

void WaveEffect::randomizeConfig(std::default_random_engine &random) {
  multiplier = std::uniform_real_distribution<float>()(random);
  amplitude = std::uniform_real_distribution<float>()(random);
}

void WaveEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    // goes from 0 (leftmost, begin) to 2 (leftmost, end)
    // but `reached` + `notOver` clamp it to 0 to 1
    float x = 2 * ${time} - initialPosition.x;

    float ease = 1.;
    if ((${rep} == 0 && x <= 0.5) || (${rep} == ${repetitions} - 1 && x >= 0.5)) {
      // The ease function is a cos spanning two negative peaks with a positive peak
      // in between. This is is then translated (+1, /2) to go from 0 to 1
      // Finally, because this will lower the actual peak height of `curve`
      // a compensation factor of 1.25 is applied
      ease = (cos((x * 2 - 1) * PI) + 1) * .5 * 1.25;
    }

    // Closed formula (with ease): (cos((x*2-1)*π)+1)/2 * sin(x*3*π-0.5*π)/0.8
    float curve = sin(x * ${multiplier} * 3. * PI - .5 * PI);
    float phase = ease * curve;
    float reached = (x >= 0.) ? 1. : 0.;
    if (${rep} != 0) {
      reached = 1.;
    }
    float notOver = (x <= 1.) ? 1. : 0.;
    if (${rep} != ${repetitions} - 1) {
      notOver = 1.;
    }

    position.y += phase * reached * notOver * ${amplitude};
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "time", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            glm::fract((props.state.clock.getTime() -
                                   timeBegin) / getPeriod()));
                      }),
              UNIFORM(data.uniforms, "rep", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            std::floor((props.state.clock.getTime() -
                                  timeBegin) / getPeriod()));
                      }),
              UNIFORM(data.uniforms, "multiplier", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(multiplier);
                      }),
              UNIFORM(data.uniforms, "amplitude", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(amplitude);
                      }),
              UNIFORM(data.uniforms, "repetitions", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<float>(repetitions));
                      }),
          })
          .c_str());
}

void WaveEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
  if(amplitude > .1f /*&& std::bernoulli_distribution()(data.random)*/) {
    if(getPeriod() >= 2500.f) {
      data.soundPlaylist.add(data.sampleLibrary->getSample("sweep005"), timeBegin);
    } else {
      data.soundPlaylist.add(data.sampleLibrary->getSample("up_sweep018"), timeBegin);
    }
  }
}
