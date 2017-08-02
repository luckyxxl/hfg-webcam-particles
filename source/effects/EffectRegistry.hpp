#pragma once

#include "../ShaderBuilder.hpp"

class IEffect {
  public:
  virtual const char *getName() const = 0;
  virtual const char *getDescriptiveName() const = 0;
  virtual const char *getDescription() const = 0;

  struct IConfig {
    virtual void load(const json &json) = 0;
    virtual void save(json &json) const = 0;
  };

  virtual std::unique_ptr<IConfig> getDefaultConfig() const = 0;
  virtual std::unique_ptr<IConfig> getRandomConfig() const = 0;

  virtual void registerEffect(const EffectInstance &instance, Uniforms &uniforms, ShaderBuilder &vertexShader, ShaderBuilder &fragmentShader) const = 0;
};

struct EffectInstance {
  const IEffect *effect;
  std::unique_ptr<IEffect::IConfig> config;

  float timeBegin = 0.f;
  float timeEnd = 1.f;
  unsigned repetitions = 1u;

  float getPeriod() const {
    return (timeEnd - timeBegin) / repetitions;
  }

  void load(const json &json);
  void save(json &json) const;
};

class EffectRegistry {
  public:
  template<class Effect>
  void registerEffect() {
    effects.emplace_back(std::make_unique<Effect>());
  }

  std::unique_ptr<EffectInstance> createInstance(const char *effectName) const;

  private:
  std::vector<std::unique_ptr<IEffect>> effects;
};
