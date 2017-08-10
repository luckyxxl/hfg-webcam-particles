#pragma once

#include "sound/Renderer.hpp"
#include "RendererState.hpp"

class SoundPlaylist {
public:
  void clear();
  void add(const sound::SampleBuffer *sample, double startTime);

  void update(const Clock &clock, sound::Renderer *renderer);

private:
  struct Item {
    double startTime;
    const sound::SampleBuffer *sample;

    friend bool operator<(const Item &a, const Item &b) {
      return a.startTime < b.startTime;
    }
    friend bool operator<(const Item &a, const double &b) {
      return a.startTime < b;
    }
    friend bool operator<(const double &a, const Item &b) {
      return a < b.startTime;
    }
  };

  // <3
  std::multiset<Item, std::less<>> items;
};
