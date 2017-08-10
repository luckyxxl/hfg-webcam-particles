#pragma once

#include "EffectRegistry.hpp"

class WaveEffect : public IEffect {
public:
  static constexpr auto Name = "WaveEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;
  void registerEffectSound(EffectSoundRegistrationData &data) const override;

public:
  float multiplier = 1.f;
  float amplitude = .05f;
};
