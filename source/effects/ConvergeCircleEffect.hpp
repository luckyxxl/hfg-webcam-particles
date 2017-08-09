#pragma once

#include "EffectRegistry.hpp"

class ConvergeCircleEffect : public IEffect {
public:
  static constexpr auto Name = "ConvergeCircleEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;

public:
  float rotationSpeed = 0.f;
};
