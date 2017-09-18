#include "main.hpp"

#include "IntervalMath.hpp"

TEST_CASE("IntervalMath - getEmptyIntervals") {
  std::vector<Interval> intervals;
  intervals.emplace_back(1.f, 2.f);
  intervals.emplace_back(1.f, 3.f);
  intervals.emplace_back(1.5f, 2.f);
  intervals.emplace_back(1.5f, 2.5f);

  intervals.emplace_back(10.f, 12.f);
  intervals.emplace_back(10.f, 13.f);
  intervals.emplace_back(10.5f, 12.f);
  intervals.emplace_back(10.5f, 12.5f);

  auto emptyIntervals = getEmptyIntervals(intervals, 0.f, 20.f);
  REQUIRE(emptyIntervals.size() == 3u);
  CHECK(emptyIntervals[0].start() == 0.f);
  CHECK(emptyIntervals[0].end() == 1.f);
  CHECK(emptyIntervals[1].start() == 3.f);
  CHECK(emptyIntervals[1].end() == 10.f);
  CHECK(emptyIntervals[2].start() == 13.f);
  CHECK(emptyIntervals[2].end() == 20.f);
}

TEST_CASE("IntervalMath - getEmptyIntervals without outer range") {
  std::vector<Interval> intervals;
  intervals.emplace_back(1.f, 2.f);
  intervals.emplace_back(1.f, 3.f);
  intervals.emplace_back(1.5f, 2.f);
  intervals.emplace_back(1.5f, 2.5f);

  intervals.emplace_back(10.f, 12.f);
  intervals.emplace_back(10.f, 13.f);
  intervals.emplace_back(10.5f, 12.f);
  intervals.emplace_back(10.5f, 12.5f);

  auto emptyIntervals = getEmptyIntervals(intervals);
  REQUIRE(emptyIntervals.size() == 1u);
  CHECK(emptyIntervals[0].start() == 3.f);
  CHECK(emptyIntervals[0].end() == 10.f);
}
