#include "main.hpp"

#include "EffectRegistry.hpp"

void IEffect::loadInstanceConfig(const json &json) {
  timeBegin = json.value("timeBegin", 0.f);
  timeEnd = json.value("timeEnd", 1000.f);
  repetitions = json.value("repetitions", 1u);
  enabled = json.value("enabled", true);
  loadConfig(json.value("config", json::object()));
}

void IEffect::saveInstanceConfig(json &json) const {
  json.emplace("timeBegin", timeBegin);
  json.emplace("timeEnd", timeEnd);
  json.emplace("repetitions", repetitions);
  json.emplace("enabled", enabled);
  json.emplace("config", json::object());
  saveConfig(json["config"]);
}
