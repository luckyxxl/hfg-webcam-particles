#include "main.hpp"

#include "VignetteEffect.hpp"

constexpr const char *VignetteEffect::Name;

const char *VignetteEffect::getName() const { return Name; }
const char *VignetteEffect::getDescriptiveName() const {
  return "Vignette";
}
const char *VignetteEffect::getDescription() const {
  return "Applies a vignette effect to the overall particle image";
}

void VignetteEffect::loadConfig(const json &json) {
  loadEaseInOutConfig(json);
}
void VignetteEffect::saveConfig(json &json) const {
  saveEaseInOutConfig(json);
}

void VignetteEffect::randomizeConfig(std::default_random_engine &random) {
}

void VignetteEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    vec2 screenCoord = texcoord * 2.0 - 1.0;

    float a = atan(screenCoord.y, screenCoord.x);

    // https://en.wikipedia.org/wiki/Squircle
    // https://thatsmaths.com/2016/07/14/squircles/  (Eq. 3)
    float s = sin(2 * a);
    float x = length(screenCoord) - s * s * mix(1.0, 0.1, ${factor});

    const float blendStart = 0.8;
    const float blendEnd = 1.0;

    // linear ramp from blend start to blend end
    float l = clamp((x - blendStart) * (1.0 / (blendEnd - blendStart)), 0.0, 1.0);

    visibility *= mix(1.0, 1.0 - l*l, ${factor});
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "factor", GLSLType::Float,
                      [this](const RenderProps &props) {
                        auto result = getEase(props);
                        return UniformValue(result);
                      }),
          })
          .c_str());
}

void VignetteEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
