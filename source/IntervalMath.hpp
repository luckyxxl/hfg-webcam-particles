#pragma once

struct Interval : std::tuple<float, float> {
  Interval(float start, float end) : std::tuple<float, float>(start, end) {
    assert(start <= end);
  }

  float start() const { return std::get<0>(*this); }
  float end() const { return std::get<1>(*this); }
  float& start() { return std::get<0>(*this); }
  float& end() { return std::get<1>(*this); }

  float length() const {
    return end() - start();
  }
};

static std::vector<Interval> getEmptyIntervals(const std::vector<Interval> &intervals,
    const float outerStart = NAN, const float outerEnd = NAN) {
  assert(std::is_sorted(intervals.begin(), intervals.end()));

  std::vector<Interval> emptyIntervals;

  auto it = intervals.begin();

  if(!std::isnan(outerStart) && it->start() > outerStart) {
    emptyIntervals.emplace_back(outerStart, it->start());
  }

  loop: {
    auto it2 = it;
    while(++it2 != intervals.end()) {
      if(it2->start() < it->end() && it2->end() > it->end()) it = it2;
      else if(it2->start() > it->end()) break;
    }

    if(it2 != intervals.end()) {
      emptyIntervals.emplace_back(it->end(), it2->start());
      it = it2;
      goto loop;
    }
  }

  if(!std::isnan(outerEnd) && it->end() < outerEnd) {
    emptyIntervals.emplace_back(it->end(), outerEnd);
  }

  return std::move(emptyIntervals);
}
