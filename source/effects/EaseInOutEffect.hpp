#pragma once

#include "EffectRegistry.hpp"

class IEaseInOutEffect : public IEffect {
public:
  enum class EaseFunction {
    None,
    Linear,
    Pow1_2,
    Pow2,
    SineInOut,
    SineIn,
    SineOut,
  };

  float easeInTime = 1000.f;
  EaseFunction easeInFunction = EaseFunction::Linear;
  float easeOutTime = 1000.f;
  EaseFunction easeOutFunction = EaseFunction::Linear;

protected:
  void loadEaseInOutConfig(const json &json);
  void saveEaseInOutConfig(json &json) const;

  float getEase(const RenderProps &props) const;
};
