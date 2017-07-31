#pragma once

class IEffect {
  public:
  virtual const char *getName() const = 0;
  virtual const char *getDescriptiveName() const = 0;
  virtual const char *getDescription() const = 0;

  virtual void randomizeConfig() = 0;

  virtual void loadConfig(const json &json) = 0;
  virtual void saveConfig(json &json) const = 0;

  virtual void writeVertexShader() const = 0;
  //virtual void writeFragmentShader() const = 0;
  //virtual void scheduleSound() const = 0;

  protected:
  float timeBegin = 0.f;
  float timeEnd = 1.f;
  unsigned repetitions = 1u;

  void loadInstanceConfig(const json &json);
  void saveInstanceConfig(json &json) const;
};

class EffectRegistry {
  public:
  template<class Effect>
  void registerEffect() {
    effects.emplace(Effect::Name, std::make_unique<Effect>);
  }

  private:
  std::map<std::string, std::function<std::unique_ptr<IEffect>()>> effects;
};
