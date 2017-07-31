#pragma once

#include "EffectRegistry.hpp"

class ConvergeCircleEffect : public IEffect {
  public:
  static constexpr auto Name = "ConvergeCircle";

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
  float rotationSpeed;
};
