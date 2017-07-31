#pragma once

class IEffect {
  public:
  virtual const char *getName() const = 0;
  virtual const char *getDescriptiveName() const = 0;
  virtual const char *getDescription() const = 0;

  class IConfig {
    public:
    virtual void load(const json &json) = 0;
    virtual void save(json &json) const = 0;
  };

  virtual std::unique_ptr<IConfig> getDefaultConfig() const = 0;
  virtual std::unique_ptr<IConfig> getRandomConfig() const = 0;

  virtual void writeVertexShader(const IConfig *config) const = 0;
  //virtual void writeFragmentShader(const IConfig *config) const = 0;
  //virtual void scheduleSound(const IConfig *config) const = 0;
};

class EffectRegistry {
  public:
  template<class Effect>
  void registerEffect() {
    effects.push_back(std::make_unique<Effect>());
  }

  const IEffect *getEffectByName(const char *name) const;

  private:
  std::vector<std::unique_ptr<IEffect>> effects;
};
