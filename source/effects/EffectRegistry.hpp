#pragma once

struct EffectInstance;

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

  virtual void writeVertexShader(const EffectInstance &instance) const = 0;
  virtual void writeFragmentShader(const EffectInstance &instance) const = 0;
  virtual void scheduleSound(const EffectInstance &instance) const = 0;
};

struct EffectInstance {
  const IEffect *effect;
  std::unique_ptr<IEffect::IConfig> config;

  float timeBegin = 0.f;
  float timeEnd = 1.f;
  unsigned repetitions = 1u;

  void load(const json &json);
  void save(json &json) const;
};

class EffectRegistry {
  public:
  template<class Effect>
  void registerEffect() {
    effects.emplace_back(std::make_unique<Effect>());
  }

  private:
  std::vector<std::unique_ptr<IEffect>> effects;
};
