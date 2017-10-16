#include "main.hpp"

#include "ReactionTimelineRandomizer.hpp"

#include "effects/AllEffects.hpp"
#include "IntervalMath.hpp"

std::unique_ptr<Timeline> ReactionTimelineRandomizer::createTimeline(EffectRegistry *effectRegistry) {
  auto timeline = std::make_unique<Timeline>(effectRegistry);

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

  // whole show
  {
    auto reduceCount = timeline->emplaceEffectInstance<ReduceParticleCountEffect>();
    reduceCount->amount = 256u;
    reduceCount->easeInTime = 1000.f;
    reduceCount->easeOutTime = 1000.f;
    reduceCount->easeFunc = ReduceParticleCountEffect::EaseFunction::Linear;
    wholeShowEffectInstances.push_back(reduceCount);

    auto sizeModify = timeline->emplaceEffectInstance<ParticleSizeModifyEffect>();
    sizeModify->scaling = 4.f;
    sizeModify->easeInTime = 1000.f;
    sizeModify->easeOutTime = 1000.f;
    sizeModify->easeFunc = ParticleSizeModifyEffect::EaseFunction::Linear;
    wholeShowEffectInstances.push_back(sizeModify);

#if 0
    auto accum = timeline->emplaceEffectInstance<TrailsEffect>();
    accum->fadeIn = 1000.f;
    accum->fadeOut = 1000.f;
    accum->strength = .8f;
    wholeShowEffectInstances.push_back(accum);
#endif
  }

#if 0
  // fade in
  {
    auto displace = timeline->emplaceEffectInstance<HueDisplace2Effect>(1u);
    displace->timeBegin = 0.f;
    displace->timeEnd = fadeInTime + 3000.f;
    displace->easeInTime = fadeInTime;
    displace->easeInFunction = IEaseInOutEffect::EaseFunction::Linear;
    displace->easeOutTime = 3000.f;
    displace->easeOutFunction = IEaseInOutEffect::EaseFunction::Linear;
    displace->distance = .2f;
    displace->scaleByValue = .5f;
    displace->rotate = .2f;
    fadeInEffectInstances.push_back(displace);

    auto converge = timeline->emplaceEffectInstance<ConvergePoint2Effect>(1u);
    converge->timeBegin = 0.f;
    converge->timeEnd = fadeInTime + 3000.f;
    converge->easeInTime = fadeInTime - 10000.f;
    converge->easeInFunction = IEaseInOutEffect::EaseFunction::Linear;
    converge->easeOutTime = 3000.f;
    converge->easeOutFunction = IEaseInOutEffect::EaseFunction::Linear;
    fadeInEffectInstances.push_back(converge);
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

  auto randomLength = std::uniform_real_distribution<float>(15000.f, 30000.f)(random);

  // randomize effect time ranges
  for(auto i : randomEffectInstances) {
    const auto minInstanceLength = 5000.f;

    i->timeBegin = std::uniform_real_distribution<float>(0.f, randomLength - minInstanceLength)(random);
    i->timeEnd = i->timeBegin + std::uniform_real_distribution<float>(minInstanceLength, randomLength - i->timeBegin)(random);
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

    auto emptyIntervals = getEmptyIntervals(intervals, 0.f);

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
      if(!isEmptyEffect(i)) randomLength = std::max(randomLength, i->timeEnd);
    }

    for(auto i : randomEffectInstances) {
      if(isEmptyEffect(i)) {
        i->timeBegin = std::max(i->timeBegin, 0.f);
        i->timeEnd = std::min(i->timeEnd, randomLength);
      }
    }
  }

  // randomize effect configs
  for(auto i : randomEffectInstances) {
    i->randomizeConfig(random);
  }


  for(auto i : wholeShowEffectInstances) {
    i->timeBegin = 0.f;
    i->timeEnd = randomLength;
  }
}

