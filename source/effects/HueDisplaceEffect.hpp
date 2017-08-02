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

    float randomDirectionOffsetValue = NAN; // updated during rendering
  };

  std::unique_ptr<IConfig> getDefaultConfig() const override;
  std::unique_ptr<IConfig> getRandomConfig() const override;

  void registerEffect(const EffectInstance &instance, Uniforms &uniforms, ShaderBuilder &vertexShader, ShaderBuilder &fragmentShader) const override;
};
