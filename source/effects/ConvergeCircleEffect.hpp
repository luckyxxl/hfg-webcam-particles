#pragma once

#include "EffectRegistry.hpp"

class ConvergeCircleEffect : public IEffect {
  public:
  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  class Config : public IConfig {
    float rotationSpeed;

    void load(const json &json) override;
    void save(json &json) const override;
  };

  std::unique_ptr<IConfig> getDefaultConfig() const override;
  std::unique_ptr<IConfig> getRandomConfig() const override;

  void writeVertexShader(const IConfig *config) const override;
  //void writeFragmentShader(const IConfig *config) const override;
  //void scheduleSound(const IConfig *config) const override;
};
