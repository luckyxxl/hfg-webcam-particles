#pragma once

#include "Timeline.hpp"
#include "effects/EffectRegistry.hpp"

class ReduceParticleCountEffect;
class ParticleSizeModifyEffect;
class HueDisplace2Effect;
class ConvergePoint2Effect;
class ConvergeCircle2Effect;

class ReactionTimelineRandomizer {
public:
  std::unique_ptr<Timeline> createTimeline(EffectRegistry *effectRegistry);
  void randomize(std::default_random_engine &random);

#if WITH_EDIT_TOOLS
  void editUpdate();
#endif

private:
  struct {
    ReduceParticleCountEffect *reduceCount;
    ParticleSizeModifyEffect *sizeModify;
    ConvergeCircle2Effect *convergeCircle;
  } wholeShowEffects;

  struct {
    HueDisplace2Effect *displace;
    HueDisplace2Effect *displace2;
    ConvergePoint2Effect *convergePoint;
    ConvergeCircle2Effect *convergeCircle;
  } fadeInEffects;

  struct WholeShowElement {
    IEffect *i;
    const float &timeOffset;
  };
  std::vector<WholeShowElement> wholeShowEffectInstances;

  struct RandomElement {
    IEffect *i;
  };
  std::vector<RandomElement> randomEffectInstances;

  struct FadeInElement {
    IEffect *i;
    const float &timeBeginOffset;
    const float &timeEndOffset;
  };
  std::vector<FadeInElement> fadeInEffectInstances;

  struct FadeOutElement {
    IEffect *i;
    const FadeInElement *base;
  };
  std::vector<FadeOutElement> fadeOutEffectInstances;
  void mirrorFadeOutEffects();
};

