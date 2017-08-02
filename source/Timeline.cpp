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
      instance->loadInstanceConfig(jsonItem);

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
      jsonItem["id"] = item->getName();
      item->saveInstanceConfig(jsonItem);

      jsonTrack.push_back(jsonItem);
    }

    json.push_back(jsonTrack);
  }
}
