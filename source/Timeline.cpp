#include "main.hpp"

#include "Timeline.hpp"

Timeline::Timeline(EffectRegistry *effectRegistry) : effectRegistry(effectRegistry) {
}

void Timeline::load(const json &json) {
  for(auto &jsonTrack : json) {
    Track track;

    for(auto &jsonItem : jsonTrack) {
      auto id = jsonItem.at("id").get<std::string>();
      auto instance = effectRegistry->createInstance(id.c_str());
      instance->load(jsonItem);

      track.push_back(std::move(instance));
    }

    tracks.push_back(std::move(track));
  }
}

void Timeline::save(json &json) const {
  for(auto &track : tracks) {
    auto jsonTrack = json::array();

    for(auto &item : track) {
      auto jsonItem = json::object();
      jsonItem["id"] = item->effect->getName();
      item->save(jsonItem);

      jsonTrack.push_back(jsonItem);
    }

    json.push_back(jsonTrack);
  }
}

void Timeline::forEachInstance(std::function<void(const EffectInstance&)> f) const {
  for(auto &track : tracks) for(auto &item : track) {
    f(*item);
  }
}
