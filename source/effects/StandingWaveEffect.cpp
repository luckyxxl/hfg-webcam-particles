#include "main.hpp"

#include "StandingWaveEffect.hpp"

constexpr const char *StandingWaveEffect::Name;

const char *StandingWaveEffect::getName() const { return Name; }
const char *StandingWaveEffect::getDescriptiveName() const {
  return "Standing Wave";
}
const char *StandingWaveEffect::getDescription() const {
  return "A standing wave oscillates";
}

// TODO: static_assert that the strings are correct (at least the count)
static auto DimensionStrings = {
  "x",
  "y",
};
static auto TimeInterpolationStrings = {
  "sine",
  "linear",
};
static auto WaveFunctionStrings = {
  "sine",
  "triangle",
};

template<class T>
T jsonEnumValue(const json &json, const char *member, T defaultValue, const std::initializer_list<const char*> &strings) {
  auto string = json.value(member, "");
  if(!string.empty()) {
    for(uint32_t i=0u; i<strings.size(); ++i) {
      if(string == *(strings.begin() + i)) {
        return static_cast<T>(i);
      }
    }
  }
  return defaultValue;
}

template<class T>
void jsonEnumEmplace(json &json, const char *member, T value, const std::initializer_list<const char*> &strings) {
  json.emplace(member, *(strings.begin() + static_cast<int>(value)));
}

void StandingWaveEffect::loadConfig(const json &json) {
  maxAmplitude = json.value("maxAmplitude", .05f);
  waveCount = json.value("waveCount", 20u);
  dimension = jsonEnumValue(json, "dimension", Dimension::X, DimensionStrings);
  timeInterpolation = jsonEnumValue(json, "timeInterpolation", TimeInterpolation::Sine, TimeInterpolationStrings);
  waveFunction = jsonEnumValue(json, "waveFunction", WaveFunction::Sine, WaveFunctionStrings);
}
void StandingWaveEffect::saveConfig(json &json) const {
  json.emplace("maxAmplitude", maxAmplitude);
  json.emplace("waveCount", waveCount);
  jsonEnumEmplace(json, "dimension", dimension, DimensionStrings);
  jsonEnumEmplace(json, "timeInterpolation", timeInterpolation, TimeInterpolationStrings);
  jsonEnumEmplace(json, "waveFunction", waveFunction, WaveFunctionStrings);
}

void StandingWaveEffect::randomizeConfig(std::default_random_engine &random) {
  maxAmplitude = std::uniform_real_distribution<float>(0.f, .2f)(random);
  waveCount = std::uniform_int_distribution<uint32_t>(1, 30)(random);
  dimension = static_cast<Dimension>(std::uniform_int_distribution<int>(0, 1)(random));
  timeInterpolation = static_cast<TimeInterpolation>(std::uniform_int_distribution<int>(0, 1)(random));
  waveFunction = static_cast<WaveFunction>(std::uniform_int_distribution<int>(0, 1)(random));
}

void StandingWaveEffect::registerEffect(EffectRegistrationData &data) const {
  data.vertexShader.appendMainBody(
      TEMPLATE(R"glsl(
  {
    int dim = ${dimension};
    int otherDim = dim == 0 ? 1 : 0;

    float t = ${time};
    float x = initialPosition[otherDim] * ${waveCount};

    float timeAmp = 0.;
    switch(${timeInterpolationFunc}) {
      case 0: // linear
      // 'linear' is a triangle function that interpolates the points (0,0),(0.25,1),(0.5,0),(0.75,-1),(1,0)
      // i.e. |/\___
      //      |  \/
      timeAmp = abs(fract(t + 0.75) - 0.5) * 4. - 1.;
      break;
      case 1: // sine
      timeAmp = sin(t * 2. * PI);
      break;
    }

    float posAmp = 0.;
    switch(${waveFunc}) {
      case 0: // linear
      posAmp = abs(fract(x + 0.75) - 0.5) * 4. - 1.;
      break;
      case 1: // sine
      posAmp = sin(x * 2. * PI);
      break;
    }

    float amplitude = ${maxAmplitude} * posAmp * timeAmp;

    position[dim] += amplitude;
  }
  )glsl")
          .compile({
            UNIFORM(data.uniforms, "time", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(
                            glm::fract((props.state.clock.getTime() -
                                   timeBegin) / getPeriod()));
                      }),
            UNIFORM(data.uniforms, "maxAmplitude", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(maxAmplitude);
                      }),
            UNIFORM(data.uniforms, "waveCount", GLSLType::Float,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<float>(waveCount));
                      }),
            UNIFORM(data.uniforms, "dimension", GLSLType::Int,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<int>(dimension));
                      }),
            UNIFORM(data.uniforms, "timeInterpolationFunc", GLSLType::Int,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<int>(timeInterpolation));
                      }),
            UNIFORM(data.uniforms, "waveFunc", GLSLType::Int,
                      [this](const RenderProps &props) {
                        return UniformValue(static_cast<int>(waveFunction));
                      }),
          })
          .c_str());
}

void StandingWaveEffect::registerEffectSound(EffectSoundRegistrationData &data) const {
}
