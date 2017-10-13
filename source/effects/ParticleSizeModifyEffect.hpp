#pragma once

#include "EffectRegistry.hpp"

class ParticleSizeModifyEffect : public IEffect {
public:
  static constexpr auto Name = "ParticleSizeModifyEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;
  void registerEffectSound(EffectSoundRegistrationData &data) const override;

public:
  float scaling = 1.f;
  float easeInTime = 1000.f;
  float easeOutTime = 1000.f;
  enum class EaseFunction {
    Sine,
    Linear,
    None,
  } easeFunc = EaseFunction::Sine;
};
