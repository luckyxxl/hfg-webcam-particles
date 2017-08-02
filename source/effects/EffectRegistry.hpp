#pragma once

#include "../ShaderBuilder.hpp"
#include "../RendererState.hpp"

class IEffect {
  public:
  virtual const char *getName() const = 0;
  virtual const char *getDescriptiveName() const = 0;
  virtual const char *getDescription() const = 0;

  virtual void loadConfig(const json &json) = 0;
  virtual void saveConfig(json &json) const = 0;

  virtual void randomizeConfig() = 0;

  virtual void registerEffect(Uniforms &uniforms, ShaderBuilder &vertexShader, ShaderBuilder &fragmentShader) const = 0;

  void loadInstanceConfig(const json &json);
  void saveInstanceConfig(json &json) const;

  protected:
  float timeBegin = 0.f;
  float timeEnd = 1.f;
  unsigned repetitions = 1u;

  float getPeriod() const {
    return (timeEnd - timeBegin) / repetitions;
  }
};

class EffectRegistry {
  public:
  template<class Effect>
  void registerEffect() {
    effects.emplace(Effect::Name, std::make_unique<Effect>);
  }

  std::unique_ptr<IEffect> createInstance(const char *effectName) const {
    return effects.at(effectName)();
  }

  private:
  std::map<std::string, std::function<std::unique_ptr<IEffect>()>> effects;
};
