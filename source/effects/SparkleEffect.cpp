#include "main.hpp"

#include "SparkleEffect.hpp"

constexpr const char *SparkleEffect::Name;

const char *SparkleEffect::getName() const { return Name; }
const char *SparkleEffect::getDescriptiveName() const {
  return "Sparkle";
}
const char *SparkleEffect::getDescription() const {
  return "Particle size and brightness increase randomly";
}

void SparkleEffect::loadConfig(const json &json) {
  scaleMin = json.value("scaleMin", .5f);
  scaleMax = json.value("scaleMax", 1.5f);
  ratio = json.value("ratio", .7f);
  duration = json.value("duration", 700.f);
}
void SparkleEffect::saveConfig(json &json) const {
  json.emplace("scaleMin", scaleMin);
  json.emplace("scaleMax", scaleMax);
  json.emplace("ratio", ratio);
  json.emplace("duration", duration);
}

void SparkleEffect::randomizeConfig(std::default_random_engine &random) {
  scaleMin = std::uniform_real_distribution<float>(0.2f, 1.f)(random);
  scaleMax = std::uniform_real_distribution<float>(1.f, 5.f)(random);
  ratio = std::uniform_real_distribution<float>(0.f, 1.f)(random);
  duration = std::uniform_real_distribution<float>(500.f, 3000.f)(random);
}

void SparkleEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    float progressFun = 1.0;
    pointSize += progressFun * ${ease} * particleSize;
    //color += progressFun * ${ease};
  }
  )glsl")
          .compile({
              UNIFORM(data.uniforms, "ease", GLSLType::Float,
                      [this](const RenderProps &props) {
                        const auto easeInTime = 1000.f;
                        const auto easeOutTime = 1000.f;
                        const auto time = glm::fract((props.state.clock.getTime() - timeBegin) / getPeriod());
                        const auto easeInProgress = glm::min(1.f, time / (easeInTime / getPeriod()));
                        const auto easeOutProgress = glm::min(1.f, (1.f - time) / (easeOutTime / getPeriod()));
                        return UniformValue(glm::min(easeInProgress, easeOutProgress));
                      }),
          })
          .c_str());
}

void SparkleEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
