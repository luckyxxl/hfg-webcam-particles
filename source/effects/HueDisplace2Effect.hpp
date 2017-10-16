#pragma once

#include "EaseInOutEffect.hpp"

class HueDisplace2Effect : public IEaseInOutEffect {
public:
  static constexpr auto Name = "HueDisplace2Effect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;
  void registerEffectSound(EffectSoundRegistrationData &data) const override;

public:
  float distance = 0.f;
  float scaleByValue = 0.f;
  float directionOffset = 0.f;
  float rotate = 0.f;
};
