#pragma once

#include "../RendererState.hpp"
#include "../ShaderBuilder.hpp"
#include "../SampleLibrary.hpp"
#include "../SoundPlaylist.hpp"

struct EffectRegistrationData {
  Uniforms &uniforms;
  ShaderBuilder &vertexShader;
  ShaderBuilder &fragmentShader;

  Uniforms &accumulationUniforms;
  ShaderBuilder &accumulationShader;

  EffectRegistrationData(Uniforms &uniforms, ShaderBuilder &vertexShader,
                         ShaderBuilder &fragmentShader,
                         Uniforms &accumulationUniforms,
                         ShaderBuilder &accumulationShader)
                         : uniforms(uniforms), vertexShader(vertexShader),
                           fragmentShader(fragmentShader),
                           accumulationUniforms(accumulationUniforms),
                           accumulationShader(accumulationShader) {}
};

struct EffectSoundRegistrationData {
  SoundPlaylist &soundPlaylist;

  const SampleLibrary *sampleLibrary;

  std::default_random_engine &random;

  EffectSoundRegistrationData(SoundPlaylist &soundPlaylist,
                              const SampleLibrary *sampleLibrary,
                              std::default_random_engine &random)
                              : soundPlaylist(soundPlaylist),
                                sampleLibrary(sampleLibrary),
                                random(random) {}
};

class IEffect {
public:
  virtual const char *getName() const = 0;
  virtual const char *getDescriptiveName() const = 0;
  virtual const char *getDescription() const = 0;

  virtual void loadConfig(const json &json) = 0;
  virtual void saveConfig(json &json) const = 0;

  virtual void randomizeConfig(std::default_random_engine &random) = 0;

  virtual void registerEffect(EffectRegistrationData &data) const = 0;
  virtual void registerEffectSound(EffectSoundRegistrationData &data) const = 0;

  virtual bool isAccumulationEffect() const { return false; }

  void loadInstanceConfig(const json &json);
  void saveInstanceConfig(json &json) const;

  float getTimeBegin() const { return timeBegin; }
  float getTimeEnd() const { return timeEnd; }

public:
  float timeBegin = 0.f;
  float timeEnd = 1000.f;
  unsigned repetitions = 1u;

  float getPeriod() const { return (timeEnd - timeBegin) / repetitions; }
};

class EffectRegistry {
public:
  template <class Effect> void registerEffect() {
    effects.emplace(Effect::Name, std::make_unique<Effect>);
  }

  std::unique_ptr<IEffect> createInstance(const char *effectName) const {
    return effects.at(effectName)();
  }

private:
  std::map<std::string, std::function<std::unique_ptr<IEffect>()>> effects;
};

template<class T>
static T jsonEnumValue(const json &json, const char *member, T defaultValue, const std::initializer_list<const char*> &strings) {
  auto string = json.value(member, "");
  if(!string.empty()) {
    for(uint32_t i=0u; i<strings.size(); ++i) {
      if(string == *(strings.begin() + i)) {
        return static_cast<T>(i);
      }
    }
  }
  return defaultValue;
}

template<class T>
static void jsonEnumEmplace(json &json, const char *member, T value, const std::initializer_list<const char*> &strings) {
  json.emplace(member, *(strings.begin() + static_cast<int>(value)));
}
