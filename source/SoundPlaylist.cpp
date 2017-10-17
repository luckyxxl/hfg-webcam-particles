#include "main.hpp"

#include "SoundPlaylist.hpp"

using namespace sound;

void SoundPlaylist::clear() {
  items.clear();
  playedUntil = 0.;
}

void SoundPlaylist::add(const sound::SampleBuffer *sample, double startTime) {
  Item newItem;
  newItem.startTime = startTime;
  newItem.sample = sample;
  items.insert(newItem);
}

void SoundPlaylist::update(const Clock &clock, sound::Renderer *renderer) {
#if !WITH_EDIT_TOOLS // Seeking breaks this. That's fine :)
  // magic numbers
  constexpr auto LOOKAHEAD = (1000. / 15.);
  constexpr auto EPS = 0.1;

  const auto now = clock.getTime();

  const auto begin = items.lower_bound(playedUntil + EPS);
  const auto end = items.upper_bound(now + LOOKAHEAD);

  for (auto it = begin; it != end; ++it) {
    renderer->play(it->sample, Renderer::PlayParameters{
                                   std::max(it->startTime - now, 0.), false});
  }
  playedUntil = now + LOOKAHEAD;
#endif
}
