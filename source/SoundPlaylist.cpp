#include "main.hpp"

#include "SoundPlaylist.hpp"

void SoundPlaylist::clear() {
  items.clear();
}

void SoundPlaylist::add(const sound::SampleBuffer *sample, double startTime) {
  Item newItem;
  newItem.startTime = startTime;
  newItem.sample = sample;
  items.insert(newItem);
}

void SoundPlaylist::update(const Clock &clock, sound::Renderer *renderer) {
  //TODO: start sounds early and with startDelay to be more independent of
  //      framerate

  const auto playItemsRange =
      std::make_pair(items.lower_bound(clock.getTime() - clock.getDelta()),
                     items.upper_bound(clock.getTime()));

  for(auto it = playItemsRange.first; it != playItemsRange.second; ++it) {
    const auto &item = *it;

    renderer->play(item.sample);
  }
}
