#include "main.hpp"

#include "EaseInOutEffect.hpp"

static auto EaseFunctionStrings = {
  "none",
  "linear",
  "pow1_2",
  "pow2",
  "sine_inout",
  "sine_in",
  "sine_out",
};

void IEaseInOutEffect::loadEaseInOutConfig(const json &json) {
  easeInTime = json.value("easeInTime", 1000.f);
  easeInFunction = jsonEnumValue(json, "easeInFunction", EaseFunction::Linear, EaseFunctionStrings);
  easeOutTime = json.value("easeOutTime", 1000.f);
  easeOutFunction = jsonEnumValue(json, "easeOutFunction", EaseFunction::Linear, EaseFunctionStrings);
}

void IEaseInOutEffect::saveEaseInOutConfig(json &json) const {
  json.emplace("easeInTime", easeInTime);
  jsonEnumEmplace(json, "easeInFunction", easeInFunction, EaseFunctionStrings);
  json.emplace("easeOutTime", easeOutTime);
  jsonEnumEmplace(json, "easeOutFunction", easeOutFunction, EaseFunctionStrings);
}

static float calcEaseFunction(IEaseInOutEffect::EaseFunction f, float t) {
  t = glm::clamp(t, 0.f, 1.f);

  switch(f) {
    case IEaseInOutEffect::EaseFunction::None:
    return 1.f;

    case IEaseInOutEffect::EaseFunction::Linear:
    return t;

    case IEaseInOutEffect::EaseFunction::Pow1_2:
    return glm::sqrt(t);

    case IEaseInOutEffect::EaseFunction::Pow2:
    return t * t;

    case IEaseInOutEffect::EaseFunction::SineInOut:
    return (-glm::cos(t * PI) + 1.f) / 2.f;

    case IEaseInOutEffect::EaseFunction::SineIn:
    return -glm::cos(t * PI / 2.f) + 1.f;

    case IEaseInOutEffect::EaseFunction::SineOut:
    return 1.f - (-glm::cos((1.f - t) * PI / 2.f) + 1.f);
  }

  // never reached
  return 1.f;
}

float IEaseInOutEffect::getEase(const RenderProps &props) const {
  const float period = getPeriod();
  const float easeInT = easeInTime / period;
  const float easeOutT = easeOutTime / period;
  const float t = glm::fract((props.state.clock.getTime() - timeBegin) / period);

  const float easeIn = calcEaseFunction(easeInFunction, (t / easeInT));
  const float easeOut = calcEaseFunction(easeOutFunction, 1.f - ((t - (1.f - easeOutT)) / easeOutT));

  return glm::min(easeIn, easeOut);
}
