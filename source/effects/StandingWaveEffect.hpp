#pragma once

#include "EffectRegistry.hpp"

class StandingWaveEffect : public IEffect {
public:
  static constexpr auto Name = "StandingWaveEffect";

  const char *getName() const override;
  const char *getDescriptiveName() const override;
  const char *getDescription() const override;

  void loadConfig(const json &json) override;
  void saveConfig(json &json) const override;

  void randomizeConfig(std::default_random_engine &random) override;

  void registerEffect(EffectRegistrationData &data) const override;
  void registerEffectSound(EffectSoundRegistrationData &data) const override;

public:
  float maxAmplitude = .05f;
  uint32_t waveCount = 20u;
  enum class Dimension {
    X,
    Y,
  } dimension = Dimension::X;
  enum class TimeInterpolation {
    Linear,
    Sine,
  } timeInterpolation = TimeInterpolation::Sine;
  enum class WaveFunction {
    Triangle,
    Sine,
  } waveFunction = WaveFunction::Sine;
};
