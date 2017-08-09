#include "main.hpp"

#include "SmoothTrailsEffect.hpp"

constexpr const char *SmoothTrailsEffect::Name;

const char *SmoothTrailsEffect::getName() const { return Name; }
const char *SmoothTrailsEffect::getDescriptiveName() const {
  return "Smooth trails";
}
const char *SmoothTrailsEffect::getDescription() const {
  return "Enables an smoother fading image echo";
}

void SmoothTrailsEffect::loadConfig(const json &json) {
  loadFadeConfig(json);
  strength = json.value("strength", .8f);
}
void SmoothTrailsEffect::saveConfig(json &json) const {
  saveFadeConfig(json);
  json.emplace("strength", strength);
}

void SmoothTrailsEffect::randomizeConfig(std::default_random_engine &random) {
  
}

void SmoothTrailsEffect::registerEffect(EffectRegistrationData &data) const {
  data.accumulationShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    vec3 color = /* texelFetch(historyTexture, ivec2(gl_FragCoord.xy), 0).rgb * .2 + */
        texelFetchOffset(historyTexture, ivec2(gl_FragCoord.xy), 0, ivec2(-1, 0)).rgb * .25 +
        texelFetchOffset(historyTexture, ivec2(gl_FragCoord.xy), 0, ivec2(+1, 0)).rgb * .25 +
        texelFetchOffset(historyTexture, ivec2(gl_FragCoord.xy), 0, ivec2(0, -1)).rgb * .25 +
        texelFetchOffset(historyTexture, ivec2(gl_FragCoord.xy), 0, ivec2(0, +1)).rgb * .25;
    accumulationEffectResult = mix(particleColor, color, ${strength});
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
