#pragma once

#include "effects/EffectRegistry.hpp"

class Timeline {
public:
  Timeline(EffectRegistry *effectRegistry);

  void load(const json &json);
  void save(json &json) const;

  using Track = std::vector<std::unique_ptr<IEffect>>;

  void addEffectInstance(std::unique_ptr<IEffect> effect, unsigned trackIndex = 0) {
    if(tracks.size() <= trackIndex) {
      tracks.resize(trackIndex + 1);
    }

    tracks[trackIndex].push_back(std::move(effect));
  }

  template<class Effect>
  Effect *emplaceEffectInstance(unsigned trackIndex = 0) {
    auto effect = std::make_unique<Effect>();
    auto result = effect.get();
    addEffectInstance(std::move(effect), trackIndex);
    return result;
  }

  float getPeriod() const {
    if(hasFixedPeriod()) {
      return fixedPeriod;
    } else {
      auto period = 0.f;
      forEachInstance([&](const IEffect &i) {
        if(i.enabled) period = std::max(period, i.getTimeEnd());
      });
      return period;
    }
  }

  void setFixedPeriod(float period) { fixedPeriod = period; }
  void unsetFixedPeriod() { fixedPeriod = NAN; }
  bool hasFixedPeriod() const { return !std::isnan(fixedPeriod); }

  template <class f_t> void forEachInstance(const f_t &f) const {
    for (auto &track : tracks)
      for (auto &item : track) {
        f(*item);
      }
  }

  template <class f_t> void forEachInstanceOnTrack(unsigned trackIndex, const f_t &f) const {
    if(trackIndex >= tracks.size()) return; // this track does not exist...
    for (auto &item : tracks[trackIndex]) {
      f(*item);
    }
  }

  size_t getInstanceCount() const {
    size_t result = 0u;
    for(auto &t : tracks) result += t.size();
    return result;
  }

  size_t getInstanceCount(unsigned trackIndex) const {
    if(trackIndex >= tracks.size()) return 0u; // this track does not exist...
    return tracks[trackIndex].size();
  }

private:
  EffectRegistry *effectRegistry;

  std::vector<Track> tracks;
  float fixedPeriod = NAN;
};
