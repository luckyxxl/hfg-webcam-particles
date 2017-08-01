#pragma once

#include "effects/EffectRegistry.hpp"

class Timeline {
  public:
  Timeline(EffectRegistry *effectRegistry);

  void load(const json &json);
  void save(json &json) const;

  using Track = std::vector<std::unique_ptr<EffectInstance>>;

  void forEachInstance(std::function<void(const EffectInstance&)> f) const;

  private:
  EffectRegistry *effectRegistry;

  std::vector<Track> tracks;
};
