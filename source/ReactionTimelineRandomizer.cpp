#include "main.hpp"

#include "ReactionTimelineRandomizer.hpp"

#include "effects/AllEffects.hpp"
#include "IntervalMath.hpp"

#if WITH_EDIT_TOOLS
#define constexpr static
#endif

constexpr auto FADE_PHASE_TIME = 10000.f;
constexpr auto FADE_PHASE_FADE_TIME = 3000.f;

#if WITH_EDIT_TOOLS
#undef constexpr
#endif

std::unique_ptr<Timeline> ReactionTimelineRandomizer::createTimeline(EffectRegistry *effectRegistry) {
  auto timeline = std::make_unique<Timeline>(effectRegistry);

#if WITH_EDIT_TOOLS
  auto bar = TwNewBar("randomizer");
  TwDefine("randomizer position='1080 0' size='200 1000'"); //TODO position[0]=1720

  TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &FADE_PHASE_TIME, "group='fade phase' label='time'");
#endif

#if 1
  // random
  {
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<ConvergeCircleEffect>());
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<ConvergePointEffect>());
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<ParticleDisplaceEffect>());
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<HueDisplaceEffect>());
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<ParticleSpacingEffect>());
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<StandingWaveEffect>());
    randomEffectInstances.push_back(timeline->emplaceEffectInstance<WaveEffect>());
  }
#endif

  // whole show
  {
    auto reduceCount = timeline->emplaceEffectInstance<ReduceParticleCountEffect>();
    reduceCount->amount = 256u;
    reduceCount->easeInTime = reduceCount->easeOutTime = 1000.f;
    reduceCount->easeFunc = ReduceParticleCountEffect::EaseFunction::Linear;
    wholeShowEffectInstances.push_back(reduceCount);

#if WITH_EDIT_TOOLS
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &reduceCount->easeInTime, "group='reduce count' label='ease in time'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &reduceCount->easeOutTime, "group='reduce count' label='ease out time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &reduceCount->easeFunc, "group='reduce count' label='ease func'");
    TwAddVarRW(bar, NULL, TW_TYPE_UINT32, &reduceCount->amount, "group='reduce count' label=amount");
#endif

    auto sizeModify = timeline->emplaceEffectInstance<ParticleSizeModifyEffect>();
    sizeModify->scaling = 4.f;
    sizeModify->easeInTime = sizeModify->easeOutTime = 1000.f;
    sizeModify->easeFunc = ParticleSizeModifyEffect::EaseFunction::Linear;
    wholeShowEffectInstances.push_back(sizeModify);

#if WITH_EDIT_TOOLS
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &sizeModify->easeInTime, "group='reduce size' label='ease in time'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &sizeModify->easeOutTime, "group='reduce size' label='ease out time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &sizeModify->easeFunc, "group='reduce size' label='ease func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &sizeModify->scaling, "group='reduce size' label=scaling");
#endif
  }

#if 1
  // fade in
  {
    auto displace = timeline->emplaceEffectInstance<HueDisplace2Effect>(1u);
    displace->easeInTime = FADE_PHASE_TIME;
    displace->easeInFunction = IEaseInOutEffect::EaseFunction::Linear;
    displace->easeOutTime = FADE_PHASE_FADE_TIME;
    displace->easeOutFunction = IEaseInOutEffect::EaseFunction::Linear;
    displace->distance = .2f;
    displace->scaleByValue = .5f;
    displace->rotate = .2f;
    fadeInEffectInstances.push_back(displace);

    auto converge = timeline->emplaceEffectInstance<ConvergePoint2Effect>(1u);
    converge->easeInTime = FADE_PHASE_TIME;
    converge->easeInFunction = IEaseInOutEffect::EaseFunction::Linear;
    converge->easeOutTime = FADE_PHASE_FADE_TIME;
    converge->easeOutFunction = IEaseInOutEffect::EaseFunction::Linear;
    fadeInEffectInstances.push_back(converge);
  }
#endif

#if 1
  // fade out
  {
    auto displace = timeline->emplaceEffectInstance<HueDisplace2Effect>(1u);
    displace->easeInTime = FADE_PHASE_FADE_TIME;
    displace->easeInFunction = IEaseInOutEffect::EaseFunction::Linear;
    displace->easeOutTime = FADE_PHASE_TIME;
    displace->easeOutFunction = IEaseInOutEffect::EaseFunction::Linear;
    displace->distance = .2f;
    displace->scaleByValue = .5f;
    displace->rotate = .2f;
    fadeOutEffectInstances.push_back(displace);

    auto converge = timeline->emplaceEffectInstance<ConvergePoint2Effect>(1u);
    converge->easeInTime = FADE_PHASE_FADE_TIME;
    converge->easeInFunction = IEaseInOutEffect::EaseFunction::Linear;
    converge->easeOutTime = FADE_PHASE_TIME;
    converge->easeOutFunction = IEaseInOutEffect::EaseFunction::Linear;
    fadeOutEffectInstances.push_back(converge);
  }
#endif

  return timeline;
}

static bool isEmptyEffect(const IEffect *i) {
  return !i->enabled || i->isAccumulationEffect()
          || !strcmp(i->getName(), "ParticleDisplaceEffect")
          || !strcmp(i->getName(), "ParticleSpacingEffect");
}

void ReactionTimelineRandomizer::randomize(std::default_random_engine &random) {
  if(randomEffectInstances.empty()) return; // fix debugging :)

  for(auto i : wholeShowEffectInstances) {
    i->timeBegin = 0.f;
    i->timeEnd = 0.f;
  }

  for(auto i : fadeInEffectInstances) {
    i->timeBegin = 0.f;
    i->timeEnd = 0.f + FADE_PHASE_TIME + FADE_PHASE_FADE_TIME;
  }


  const auto randomStart = FADE_PHASE_TIME;
  auto randomLength = std::uniform_real_distribution<float>(15000.f, 30000.f)(random);

  // randomize effect time ranges
  for(auto i : randomEffectInstances) {
    const auto minInstanceLength = 5000.f;

    i->timeBegin = randomStart + std::uniform_real_distribution<float>(0.f, randomLength - minInstanceLength)(random);
    i->timeEnd = i->timeBegin + std::uniform_real_distribution<float>(minInstanceLength, randomStart + randomLength - i->timeBegin)(random);
    i->repetitions = std::bernoulli_distribution(.3f)(random) ? std::uniform_int_distribution<int>(10, 30)(random) : 1;
  }

  // randomize effect enabled
  {
    size_t enabledCount;
    do {
      enabledCount = 0u;
      for(auto i : randomEffectInstances) {
        i->enabled = std::bernoulli_distribution(.95)(random);
        if(i->enabled) ++enabledCount;
      }
    } while(enabledCount < 2u);
  }

  // remove empty space
  {
    const auto instanceCount = randomEffectInstances.size();
    assert(instanceCount > 0u);

    std::vector<Interval> intervals;
    intervals.reserve(instanceCount);
    for(auto i : randomEffectInstances) {
      if(isEmptyEffect(i)) continue;
      intervals.emplace_back(i->timeBegin, i->timeEnd);
    }

    std::sort(intervals.begin(), intervals.end());

    auto emptyIntervals = getEmptyIntervals(intervals, randomStart, randomStart + randomLength);

    assert(std::is_sorted(emptyIntervals.begin(), emptyIntervals.end()));

    for(auto interval = emptyIntervals.begin(); interval != emptyIntervals.end(); ++interval) {
      const auto move = -interval->length();

      for(auto i : randomEffectInstances) {
        if(interval->start() <= i->timeBegin) {
          i->timeBegin += move;
          i->timeEnd += move;
        }
      }

      for(auto i = interval+1; i != emptyIntervals.end(); ++i) {
        i->start() += move;
        i->end() += move;
      }
    }
  }

  // fix empty effects and update randomLength
  {
    randomLength = 0.f;
    for(auto i : randomEffectInstances) {
      if(!isEmptyEffect(i)) randomLength = std::max(randomLength, i->timeEnd - randomStart);
    }

    for(auto i : randomEffectInstances) {
      if(isEmptyEffect(i)) {
        i->timeBegin = std::max(i->timeBegin, randomStart);
        i->timeEnd = std::min(i->timeEnd, randomStart + randomLength);
      }
    }
  }

  // randomize effect configs
  for(auto i : randomEffectInstances) {
    i->randomizeConfig(random);
  }


  for(auto i : fadeOutEffectInstances) {
    i->timeBegin = randomStart + randomLength - FADE_PHASE_FADE_TIME;
    i->timeEnd = randomStart + randomLength + FADE_PHASE_TIME;
  }

  for(auto i : wholeShowEffectInstances) {
    i->timeBegin = 0.f;
    i->timeEnd = 2 * FADE_PHASE_TIME + randomLength;
  }
}

