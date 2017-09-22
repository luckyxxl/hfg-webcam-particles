#pragma once

#include "EffectRegistry.hpp"

class SparkleEffect : public IEffect {
public:
  static constexpr auto Name = "SparkleEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;
  void registerEffectSound(EffectSoundRegistrationData &data) const override;

public:
  float scaleMin = .5f;
  float scaleMax = 1.5f;
  float ratio = .7f;
  float duration = 700.f;
};
