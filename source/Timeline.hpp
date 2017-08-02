#pragma once

#include "effects/EffectRegistry.hpp"

class Timeline {
  public:
  Timeline(EffectRegistry *effectRegistry);

  void load(const json &json);
  void save(json &json) const;

  using Track = std::vector<std::unique_ptr<IEffect>>;

  template<class f_t>
  void forEachInstance(const f_t &f) const {
    for(auto &track : tracks) for(auto &item : track) {
      f(*item);
    }
  }

  private:
  EffectRegistry *effectRegistry;

  std::vector<Track> tracks;
};
