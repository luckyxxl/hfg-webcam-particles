#pragma once

#include "EffectRegistry.hpp"

class HueDisplaceEffect : public IEffect {
  public:
  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  struct Config : IConfig {
    float distance;
    float scaleByValue;
    bool randomDirectionOffset;
    float rotate;

    void load(const json &json) override;
    void save(json &json) const override;
  };

  std::unique_ptr<IConfig> getDefaultConfig() const override;
  std::unique_ptr<IConfig> getRandomConfig() const override;

  void writeVertexShader(const EffectInstance &instance) const override;
  void writeFragmentShader(const EffectInstance &instance) const override;
  void scheduleSound(const EffectInstance &instance) const override;
};
