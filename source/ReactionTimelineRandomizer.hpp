#pragma once

#include "Timeline.hpp"
#include "effects/EffectRegistry.hpp"

class ReactionTimelineRandomizer {
public:
  std::unique_ptr<Timeline> createTimeline(EffectRegistry *effectRegistry);
  void randomize(std::default_random_engine &random);

private:
  std::vector<IEffect*> fadeInEffectInstances;
  std::vector<IEffect*> fadeOutEffectInstances;
  std::vector<IEffect*> randomEffectInstances;
  std::vector<IEffect*> wholeShowEffectInstances;
};

