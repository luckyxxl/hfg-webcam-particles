#pragma once

#include "EffectRegistry.hpp"

class HueDisplaceEffect : public IEffect {
public:
  static constexpr auto Name = "HueDisplaceEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;

public:
  float distance = 0.f;
  float scaleByValue = 0.f;
  float scaleByForegroundMask = 0.f;
  bool randomDirectionOffset = false;
  float rotate = 0.f;

private:
  float randomDirectionOffsetValue = NAN; // updated during rendering
};
