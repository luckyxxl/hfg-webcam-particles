#include "main.hpp"

#include "EffectRegistry.hpp"

void IEffect::loadInstanceConfig(const json &json) {
  timeBegin = json.value("timeBegin", 0.f);
  timeEnd = json.value("timeEnd", 1.f);
  repetitions = json.value("repetitions", 1u);
}

void IEffect::saveInstanceConfig(json &json) const {
  json.emplace("timeBegin", timeBegin);
  json.emplace("timeEnd", timeEnd);
  json.emplace("repetitions", repetitions);
}
