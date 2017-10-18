#include "main.hpp"

#include "ReactionTimelineRandomizer.hpp"

#include "effects/AllEffects.hpp"
#include "IntervalMath.hpp"

#if WITH_EDIT_TOOLS
#define constexpr static
#endif

constexpr auto WHOLE_SHOW_REDUCE_COUNT_TIME_OFFSET = 18000.f;
constexpr auto WHOLE_SHOW_SIZE_MODIFY_TIME_OFFSET = 18000.f;

constexpr auto FADE_PHASE_TIME = 28000.f;
constexpr auto FADE_PHASE_FADE_TIME = 2000.f;

constexpr auto RANDOM_PHASE_MIN_LENGTH = 15000.f;
constexpr auto RANDOM_PHASE_MAX_LENGTH = 45000.f;
constexpr auto RANDOM_PHASE_MIN_INSTANCE_LENGTH = 5000.f;
constexpr auto RANDOM_PHASE_REP_P = .3f;
constexpr auto RANDOM_PHASE_REP_MIN = 10;
constexpr auto RANDOM_PHASE_REP_MAX = 30;

constexpr auto FADE_DISPLACE_BEGIN_OFFSET = 0.f;
constexpr auto FADE_DISPLACE2_BEGIN_OFFSET = 18000.f;
constexpr auto FADE_CONVERGE_CIRCLE_BEGIN_OFFSET = 5000.f;

#if WITH_EDIT_TOOLS
#undef constexpr
#endif

std::unique_ptr<Timeline> ReactionTimelineRandomizer::createTimeline(EffectRegistry *effectRegistry) {
  auto timeline = std::make_unique<Timeline>(effectRegistry);

#if WITH_EDIT_TOOLS
  auto bar = TwNewBar("randomizer");
  TwDefine("randomizer position='1720 0' size='200 1000'");

  TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &FADE_PHASE_TIME, "group='fade phase' label='time'");

  TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &RANDOM_PHASE_MIN_LENGTH, "group='random phase' label='min length'");
  TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &RANDOM_PHASE_MAX_LENGTH, "group='random phase' label='max length'");
  TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &RANDOM_PHASE_MIN_INSTANCE_LENGTH, "group='random phase' label='min instance length'");
  TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &RANDOM_PHASE_REP_P, "group='random phase' label='rep p'");
  TwAddVarRW(bar, NULL, TW_TYPE_INT32, &RANDOM_PHASE_REP_MIN, "group='random phase' label='rep min'");
  TwAddVarRW(bar, NULL, TW_TYPE_INT32, &RANDOM_PHASE_REP_MAX, "group='random phase' label='rep max'");
#endif

  // whole show
  {
    wholeShowEffects.reduceCount = timeline->emplaceEffectInstance<ReduceParticleCountEffect>(2u);
    wholeShowEffects.reduceCount->easeInTime = wholeShowEffects.reduceCount->easeOutTime = 10000.f;
    wholeShowEffects.reduceCount->easeFunc = ReduceParticleCountEffect::EaseFunction::Linear;
    wholeShowEffects.reduceCount->amount   = 97u;
    wholeShowEffectInstances.push_back({wholeShowEffects.reduceCount, WHOLE_SHOW_REDUCE_COUNT_TIME_OFFSET});

    wholeShowEffects.sizeModify = timeline->emplaceEffectInstance<ParticleSizeModifyEffect>(2u);
    wholeShowEffects.sizeModify->easeInTime = wholeShowEffects.sizeModify->easeOutTime = 10000.f;
    wholeShowEffects.sizeModify->easeFunc = ParticleSizeModifyEffect::EaseFunction::Linear;
    wholeShowEffects.sizeModify->scaling  = 2.f;
    wholeShowEffectInstances.push_back({wholeShowEffects.sizeModify, WHOLE_SHOW_SIZE_MODIFY_TIME_OFFSET});

    wholeShowEffects.convergeCircle = timeline->emplaceEffectInstance<ConvergeCircle2Effect>(2u);
    wholeShowEffects.convergeCircle->easeInTime = wholeShowEffects.convergeCircle->easeOutTime = 2000.f;
    wholeShowEffects.convergeCircle->easeInFunction = wholeShowEffects.convergeCircle->easeOutFunction = IEaseInOutEffect::EaseFunction::SineInOut;
    wholeShowEffects.convergeCircle->radius = .5f;
    wholeShowEffectInstances.push_back({wholeShowEffects.convergeCircle, FADE_PHASE_TIME});

#if WITH_EDIT_TOOLS
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &WHOLE_SHOW_REDUCE_COUNT_TIME_OFFSET, "group='reduce count' label='time offset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &wholeShowEffects.reduceCount->easeInTime, "group='reduce count' label='ease time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &wholeShowEffects.reduceCount->easeFunc, "group='reduce count' label='ease func'");
    TwAddVarRW(bar, NULL, TW_TYPE_UINT32, &wholeShowEffects.reduceCount->amount, "group='reduce count' label=amount");

    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &WHOLE_SHOW_SIZE_MODIFY_TIME_OFFSET, "group='reduce size' label='time offset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &wholeShowEffects.sizeModify->easeInTime, "group='reduce size' label='ease time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &wholeShowEffects.sizeModify->easeFunc, "group='reduce size' label='ease func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &wholeShowEffects.sizeModify->scaling, "group='reduce size' label=scaling");

    TwDefine("randomizer/'reduce count' group='whole show'");
    TwDefine("randomizer/'reduce size' group='whole show'");
#endif
  }

  // random
  {
    randomEffectInstances.push_back({timeline->emplaceEffectInstance<ParticleDisplaceEffect>()});
    randomEffectInstances.push_back({timeline->emplaceEffectInstance<HueDisplaceEffect>()});
    randomEffectInstances.push_back({timeline->emplaceEffectInstance<HueDisplace2Effect>()});
    randomEffectInstances.push_back({timeline->emplaceEffectInstance<ParticleSpacingEffect>()});
    randomEffectInstances.push_back({timeline->emplaceEffectInstance<StandingWaveEffect>()});
    randomEffectInstances.push_back({timeline->emplaceEffectInstance<WaveEffect>()});
  }

  // glitch
  {
    glitchEffectInstances.push_back({timeline->emplaceEffectInstance<ConvergePointEffect>()});

#if WITH_EDIT_TOOLS
    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, &glitchEffectInstances[0].i->enabled, "group='glitch' label='converge point'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &glitchEffectInstances[0].i->timeBegin, "group='glitch' label='converge point tb'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &glitchEffectInstances[0].i->timeEnd, "group='glitch' label='converge point tb'");
    TwAddVarRW(bar, NULL, TW_TYPE_UINT32, &glitchEffectInstances[0].i->repetitions, "group='glitch' label='converge point r'");
#endif
  }

  // fade in
  {
    fadeInEffects.displace = timeline->emplaceEffectInstance<HueDisplace2Effect>(1u);
    fadeInEffects.displace->easeInTime = 10000.f;
    fadeInEffects.displace->easeInFunction = IEaseInOutEffect::EaseFunction::Pow1_2;
    fadeInEffects.displace->easeOutTime = FADE_PHASE_FADE_TIME;
    fadeInEffects.displace->easeOutFunction = IEaseInOutEffect::EaseFunction::SineInOut;
    fadeInEffects.displace->distance = .1f;
    fadeInEffects.displace->scaleByValue = 1.5f;
    fadeInEffects.displace->directionOffset = 0.f;
    fadeInEffects.displace->rotate = 0.f;
    fadeInEffectInstances.push_back({fadeInEffects.displace, FADE_DISPLACE_BEGIN_OFFSET, FADE_PHASE_FADE_TIME});

    fadeInEffects.displace2 = timeline->emplaceEffectInstance<HueDisplace2Effect>(1u);
    fadeInEffects.displace2->easeInTime = 5000.f;
    fadeInEffects.displace2->easeInFunction = IEaseInOutEffect::EaseFunction::SineInOut;
    fadeInEffects.displace2->easeOutTime = FADE_PHASE_FADE_TIME;
    fadeInEffects.displace2->easeOutFunction = IEaseInOutEffect::EaseFunction::SineInOut;
    fadeInEffects.displace2->distance = .03f;
    fadeInEffects.displace2->scaleByValue = .9f;
    fadeInEffects.displace2->directionOffset = PI/2.f;
    fadeInEffects.displace2->rotate = 7.f;
    fadeInEffectInstances.push_back({fadeInEffects.displace2, FADE_DISPLACE2_BEGIN_OFFSET, FADE_PHASE_FADE_TIME});

    fadeInEffects.convergeCircle = timeline->emplaceEffectInstance<ConvergeCircle2Effect>(1u);
    fadeInEffects.convergeCircle->easeInTime = 15000.f;
    fadeInEffects.convergeCircle->easeInFunction = IEaseInOutEffect::EaseFunction::SineInOut;
    fadeInEffects.convergeCircle->easeOutTime = FADE_PHASE_FADE_TIME;
    fadeInEffects.convergeCircle->easeOutFunction = IEaseInOutEffect::EaseFunction::SineInOut;
    fadeInEffects.convergeCircle->radius = .4f;
    fadeInEffects.convergeCircle->rotationSpeed = 0.f;
    fadeInEffectInstances.push_back({fadeInEffects.convergeCircle, FADE_CONVERGE_CIRCLE_BEGIN_OFFSET, FADE_PHASE_FADE_TIME});

#if WITH_EDIT_TOOLS
    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, &fadeInEffects.displace->enabled, "group='displace' label='enabled'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &FADE_DISPLACE_BEGIN_OFFSET, "group='displace' label='begin offset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace->easeInTime, "group='displace' label='ease in time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &fadeInEffects.displace->easeInFunction, "group='displace' label='ease in func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace->easeOutTime, "group='displace' label='ease out time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &fadeInEffects.displace->easeOutFunction, "group='displace' label='ease out func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace->distance, "group='displace' label='distance'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace->scaleByValue, "group='displace' label='scaleByValue'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace->directionOffset, "group='displace' label='directionOffset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace->rotate, "group='displace' label='rotate'");

    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, &fadeInEffects.displace2->enabled, "group='displace2' label='enabled'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &FADE_DISPLACE2_BEGIN_OFFSET, "group='displace2' label='begin offset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace2->easeInTime, "group='displace2' label='ease in time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &fadeInEffects.displace2->easeInFunction, "group='displace2' label='ease in func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace2->easeOutTime, "group='displace2' label='ease out time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &fadeInEffects.displace2->easeOutFunction, "group='displace2' label='ease out func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace2->distance, "group='displace2' label='distance'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace2->scaleByValue, "group='displace2' label='scaleByValue'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace2->directionOffset, "group='displace2' label='directionOffset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.displace2->rotate, "group='displace2' label='rotate'");

    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, &fadeInEffects.convergeCircle->enabled, "group='converge circle' label='enabled'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &FADE_CONVERGE_CIRCLE_BEGIN_OFFSET, "group='converge circle' label='begin offset'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.convergeCircle->easeInTime, "group='converge circle' label='ease in time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &fadeInEffects.convergeCircle->easeInFunction, "group='converge circle' label='ease in func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.convergeCircle->easeOutTime, "group='converge circle' label='ease out time'");
    TwAddVarRW(bar, NULL, TW_TYPE_INT32, &fadeInEffects.convergeCircle->easeOutFunction, "group='converge circle' label='ease out func'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.convergeCircle->radius, "group='converge circle' label='radius'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &fadeInEffects.convergeCircle->rotationSpeed, "group='converge circle' label='rotation speed'");

    TwDefine("randomizer/displace group='fade phase'");
    TwDefine("randomizer/displace2 group='fade phase'");
    TwDefine("randomizer/'converge circle' group='fade phase'");
#endif
  }

  // fade out
  {
    auto it = fadeInEffectInstances.cbegin();

    fadeOutEffectInstances.push_back({timeline->emplaceEffectInstance<HueDisplace2Effect>(1u), &*it++});
    fadeOutEffectInstances.push_back({timeline->emplaceEffectInstance<HueDisplace2Effect>(1u), &*it++});
    fadeOutEffectInstances.push_back({timeline->emplaceEffectInstance<ConvergeCircle2Effect>(1u), &*it++});

    mirrorFadeOutEffects();
  }

  for(auto i : wholeShowEffectInstances) i.i->soundEnabled = false;
  for(auto i : glitchEffectInstances) i.i->soundEnabled = false;
  for(auto i : fadeInEffectInstances) i.i->soundEnabled = false;
  for(auto i : fadeOutEffectInstances) i.i->soundEnabled = false;

  return timeline;
}

void ReactionTimelineRandomizer::mirrorFadeOutEffects() {
  for(auto &i : fadeOutEffectInstances) {
    i.i->enabled = i.base->i->enabled;

    auto easeInOut = dynamic_cast<IEaseInOutEffect*>(i.i);
    if(easeInOut) {
      auto base = dynamic_cast<IEaseInOutEffect*>(i.base->i);
      if(!base) throw "type mismatch in mirrorFadeOutEffects";

      easeInOut->easeInTime = base->easeOutTime;
      easeInOut->easeInFunction = base->easeOutFunction;
      easeInOut->easeOutTime = base->easeInTime;
      easeInOut->easeOutFunction = base->easeInFunction;
    }

    auto hueDisplace2 = dynamic_cast<HueDisplace2Effect*>(i.i);
    if(hueDisplace2) {
      auto base = dynamic_cast<HueDisplace2Effect*>(i.base->i);
      if(!base) throw "type mismatch in mirrorFadeOutEffects";

      hueDisplace2->distance = base->distance;
      hueDisplace2->scaleByValue = base->scaleByValue;
      hueDisplace2->directionOffset = base->directionOffset;
      hueDisplace2->rotate = base->rotate;
    }

    auto convergeCircle2 = dynamic_cast<ConvergeCircle2Effect*>(i.i);
    if(convergeCircle2) {
      auto base = dynamic_cast<ConvergeCircle2Effect*>(i.base->i);
      if(!base) throw "type mismatch in mirrorFadeOutEffects";

      convergeCircle2->radius = base->radius;
      convergeCircle2->rotationSpeed = base->rotationSpeed;
    }
  }
}

void ramdomizeGlitchConfig(IEffect *instance) {
  auto convergePoint = dynamic_cast<ConvergePointEffect*>(instance);
  if(convergePoint) {
  }
}

static bool isEmptyEffect(const IEffect *i) {
  return !i->enabled || i->isAccumulationEffect()
          || !strcmp(i->getName(), "ParticleDisplaceEffect")
          || !strcmp(i->getName(), "ParticleSpacingEffect");
}

ReactionTimelineRandomizer::RandomizeResult ReactionTimelineRandomizer::randomize(std::default_random_engine &random) {
  RandomizeResult result;
  if(randomEffectInstances.empty()) return result; // fix debugging :)

  result.fadeInBegin = 0.f;
  for(auto i : fadeInEffectInstances) {
    i.i->timeBegin = i.timeBeginOffset;
    i.i->timeEnd = FADE_PHASE_TIME + i.timeEndOffset;
  }


  const auto randomStart = FADE_PHASE_TIME;
  auto randomLength = std::uniform_real_distribution<float>(RANDOM_PHASE_MIN_LENGTH, RANDOM_PHASE_MAX_LENGTH)(random);

  // randomize effect time ranges
  for(auto i : randomEffectInstances) {
    const auto minInstanceLength = RANDOM_PHASE_MIN_INSTANCE_LENGTH;

    i.i->timeBegin = randomStart + std::uniform_real_distribution<float>(0.f, randomLength - minInstanceLength)(random);
    i.i->timeEnd = i.i->timeBegin + std::uniform_real_distribution<float>(minInstanceLength, randomStart + randomLength - i.i->timeBegin)(random);
  }

  // randomize effect enabled
  {
    size_t enabledCount;
    do {
      enabledCount = 0u;
      for(auto i : randomEffectInstances) {
        i.i->enabled = std::bernoulli_distribution(.95)(random);
        if(i.i->enabled) ++enabledCount;
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
      if(isEmptyEffect(i.i)) continue;
      intervals.emplace_back(i.i->timeBegin, i.i->timeEnd);
    }

    std::sort(intervals.begin(), intervals.end());

    auto emptyIntervals = getEmptyIntervals(intervals, randomStart, randomStart + randomLength);

    assert(std::is_sorted(emptyIntervals.begin(), emptyIntervals.end()));

    for(auto interval = emptyIntervals.begin(); interval != emptyIntervals.end(); ++interval) {
      const auto move = -interval->length();

      for(auto i : randomEffectInstances) {
        if(interval->start() <= i.i->timeBegin) {
          i.i->timeBegin += move;
          i.i->timeEnd += move;
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
      if(!isEmptyEffect(i.i)) randomLength = std::max(randomLength, i.i->timeEnd - randomStart);
    }

    for(auto i : randomEffectInstances) {
      if(isEmptyEffect(i.i)) {
        i.i->timeBegin = std::max(i.i->timeBegin, randomStart);
        i.i->timeEnd = std::min(i.i->timeEnd, randomStart + randomLength);
      }
    }
  }

  // randomize effect configs
  for(auto i : randomEffectInstances) {
    i.i->randomizeConfig(random);
  }


  for(auto i : glitchEffectInstances) {
    //i.i->enabled = std::bernoulli_distribution(.25)(random);
    i.i->enabled = std::bernoulli_distribution(1.)(random);

    const auto length = std::uniform_real_distribution<float>(500.f, 1000.f)(random);
    i.i->timeBegin = std::uniform_real_distribution<float>(randomStart, randomStart + randomLength - length)(random);
    i.i->timeEnd = i.i->timeBegin + length;
    i.i->repetitions = std::uniform_int_distribution<uint32_t>(1u, 5u)(random);

    ramdomizeGlitchConfig(i.i);
  }

  assert(glitchEffectInstances.size() == 1u);
  if(glitchEffectInstances[0u].i->enabled) {
    result.glitchBegin = glitchEffectInstances[0u].i->timeBegin;
    result.glitchLength = glitchEffectInstances[0u].i->timeEnd - glitchEffectInstances[0u].i->timeBegin;
  }

  result.fadeOutBegin = randomStart + randomLength;
  for(auto i : fadeOutEffectInstances) {
    i.i->timeBegin = randomStart + randomLength - i.base->timeEndOffset;
    i.i->timeEnd = randomStart + randomLength + FADE_PHASE_TIME - i.base->timeBeginOffset;
  }

  for(auto i : wholeShowEffectInstances) {
    i.i->timeBegin = i.timeOffset;
    i.i->timeEnd = 2 * FADE_PHASE_TIME + randomLength - i.timeOffset;
  }

  return result;
}

#if WITH_EDIT_TOOLS
void ReactionTimelineRandomizer::editUpdate() {
  wholeShowEffects.reduceCount->easeOutTime = wholeShowEffects.reduceCount->easeInTime;
  wholeShowEffects.sizeModify->easeOutTime = wholeShowEffects.sizeModify->easeInTime;
  mirrorFadeOutEffects();
}
#endif
