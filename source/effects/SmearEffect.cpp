#include "main.hpp"

#include "SmearEffect.hpp"

constexpr const char *SmearEffect::Name;

const char *SmearEffect::getName() const { return Name; }
const char *SmearEffect::getDescriptiveName() const {
  return "Smear";
}
const char *SmearEffect::getDescription() const {
  return "Smears the image in a circular way";
}

void SmearEffect::loadConfig(const json &json) {
  loadFadeConfig(json);
  speed = json.value("speed", 8.f);
  strength = json.value("strength", .8f);
}
void SmearEffect::saveConfig(json &json) const {
  saveFadeConfig(json);
  json.emplace("speed", speed);
  json.emplace("strength", strength);
}

void SmearEffect::randomizeConfig(std::default_random_engine &random) {
  
}

void SmearEffect::registerEffect(EffectRegistrationData &data) const {
  data.accumulationShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    vec2 texcoord = gl_FragCoord.xy * ${invScreenSize};
    vec2 smearDir = vec2(-texcoord.y + .5, texcoord.x - .5);
    vec3 color = texture2D(historyTexture, texcoord + smearDir * ${invScreenSize} * ${speed}).rgb;
    accumulationEffectResult = mix(particleColor, color, ${strength});
  }
  )glsl")
          .compile({
            UNIFORM(data.accumulationUniforms, "invScreenSize", GLSLType::Vec2,
                      [this](const RenderProps &props) {
                        return UniformValue(glm::vec2(1.f / props.screen_width, 1.f / props.screen_height));
                      }),
            UNIFORM(data.accumulationUniforms, "speed", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(speed);
                      }),
            UNIFORM(data.accumulationUniforms, "strength", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(strength);
                      }),
          })
          .c_str());
}
