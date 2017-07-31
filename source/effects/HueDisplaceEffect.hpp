#pragma once

#include "EffectRegistry.hpp"

class HueDisplaceEffect : public IEffect {
  public:
  static constexpr auto Name = "HueDisplace";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void randomizeConfig() override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void writeVertexShader() const override;
  //void writeFragmentShader() const override;
  //void scheduleSound() const override;

  protected:
  float distance;
  float scaleByValue;
  bool randomDirectionOffset;
  float rotate;
};
