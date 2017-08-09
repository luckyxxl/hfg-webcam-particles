#pragma once

#include "EffectRegistry.hpp"

class IAccumulationEffect : public IEffect {
public:
  bool isAccumulationEffect() const override { return true; }

public:
  float fadeIn = 100.f;
  float fadeOut = 500.f;

protected:
  void loadFadeConfig(const json &json);
  void saveFadeConfig(json &json) const;
};
