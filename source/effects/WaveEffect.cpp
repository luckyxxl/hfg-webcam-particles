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

void WaveEffect::randomizeConfig() {}

void WaveEffect::registerEffect(Uniforms &uniforms,
                                          ShaderBuilder &vertexShader,
                                          ShaderBuilder &fragmentShader) const {
  vertexShader.appendMainBody(
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
    if (${rep} != ${instance.repetitions} - 1) {
      notOver = 1.;
    }

    position.y += phase * reached * notOver * ${amplitude};
  }
  )glsl")
          .compile({
              UNIFORM("time", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            glm::fract((props.state.clock.getTime() -
                                   timeBegin) / getPeriod()));
                      }),
              UNIFORM("rep", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            std::floor((props.state.clock.getTime() -
                                  timeBegin) / getPeriod()));
                      }),
              UNIFORM("multiplier", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(multiplier);
                      }),
              UNIFORM("amplitude", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(amplitude);
                      }),
              UNIFORM("repetitions", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(repetitions);
                      }),
          })
          .c_str());
}
