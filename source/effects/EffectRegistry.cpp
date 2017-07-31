#include "main.hpp"

#include "EffectRegistry.hpp"

const IEffect *EffectRegistry::getEffectByName(const char *name) const {
  for(auto &e : effects) {
    if(strcmp(e->getName(), name) == 0) return e.get();
  }
  return nullptr;
}
