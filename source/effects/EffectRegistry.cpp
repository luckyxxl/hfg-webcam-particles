#include "main.hpp"

#include "EffectRegistry.hpp"

void EffectInstance::load(const json &json) {
  timeBegin = json.value("timeBegin", 0.f);
  timeEnd = json.value("timeEnd", 1.f);
  repetitions = json.value("repetitions", 1u);
  config->load(json.value("config", json::object()));
}

void EffectInstance::save(json &json) const {
  json.emplace("timeBegin", timeBegin);
  json.emplace("timeEnd", timeEnd);
  json.emplace("repetitions", repetitions);
  json.emplace("config", json::object());
  config->save(json["config"]);
}

std::unique_ptr<EffectInstance> EffectRegistry::createInstance(const char *effectName) const {
  for(auto &e : effects) {
    if(strcmp(e->getName(), effectName) == 0) {
      auto result = std::make_unique<EffectInstance>();
      result->effect = e.get();
      return result;
    }
  }
  return std::unique_ptr<EffectInstance>();
}
