#pragma once

#include "AccumulationEffect.hpp"

class SmearEffect : public IAccumulationEffect {
public:
  static constexpr auto Name = "SmearEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;
  void registerEffectSound(EffectSoundRegistrationData &data) const override;

public:
  float speed = 8.f;
  float strength = .8f;
};
