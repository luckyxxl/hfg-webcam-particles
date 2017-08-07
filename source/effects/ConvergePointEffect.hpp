#pragma once

#include "EffectRegistry.hpp"

class ConvergePointEffect : public IEffect {
public:
  static constexpr auto Name = "ConvergePointEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig() override;

  void registerEffect(Uniforms &uniforms, ShaderBuilder &vertexShader,
                      ShaderBuilder &fragmentShader) const override;

public:
};
