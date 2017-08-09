#include "main.hpp"

#include "AccumulationEffect.hpp"

void IAccumulationEffect::loadFadeConfig(const json &json) {
  fadeIn = json.value("fadeIn", 100.f);
  fadeOut = json.value("fadeOut", 500.f);
}

void IAccumulationEffect::saveFadeConfig(json &json) const {
  json.emplace("fadeIn", fadeIn);
  json.emplace("fadeOut", fadeOut);
}
