#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <thread>

template <class provider_t, class product_t> class AsyncMostRecentDataStream {
  using self_t = AsyncMostRecentDataStream<provider_t, product_t>;
  using return_t = product_t;
  std::array<return_t, 3> slots;
  std::atomic<bool> stopped = false;
  std::thread provider_thread;
  enum STATE {
    /// Slot roles encoded as New (N), Free (F), Locked (L)
    // Invariants:
    // * locked versions are always +1
    // * the position of the lock is denoted by the second and third bit
    // interpreted as unary
    // * if 4th bit is set, new and locked exist in parallel
    // * there is always at least one F slot available
    FFF = 0,
    NFF = 1, // +1 locks
    LFF = 2,
    FNF = 3, // +1 locks
    FLF = 4,
    FFN = 5, // +1 locks
    FFL = 6,

    LNF = 8,  // unlocked: 3
    LFN = 9,  // unlocked: 5
    NLF = 10, // unlocked: 1
    FLN = 11, // unlocked: 5
    NFL = 12, // unlocked: 1
    FNL = 13  // unlocked: 3
  };
  std::atomic_int_fast8_t state;
  using state_t = std::remove_reference_t<decltype(state.load())>;

  int getLockedPosition() {
    assert(state != FFF);
    auto bits = (state >> 1) & 0B11;
    assert(bits);
    return bits - 1;
  }
  STATE acquire_read_lock() {
    assert(state < LNF);
    return static_cast<STATE>(++state);
  }
  void release_read_lock() {
    if (state >= LNF) {
      switch (state) {
      case NLF:
      case NFL:
        state = NFF;
        return;
      case LNF:
      case FNL:
        state = FNF;
        return;
      case LFN:
      case FLN:
        state = FFN;
        return;
      default:
        assert(false);
      }
    } else
      --state; // don't "Free" the slot's content for we might want to re-serve
               // it in the future while there is no new content
  }
  int getWritePosition(STATE S) {
    if (S < 5)
      return 2;
    if (S < 8)
      return 0;
    switch (S) {
    case 11:
    case 13:
      return 0;
    case 9:
    case 12:
      return 1;
    case 8:
    case 10:
      return 2;
    default:
      assert(false);
    }
  }
  void commitUpdate(int pos) {
    auto getNewState = [pos](STATE S) {
      std::array<std::array<STATE, 3>, 14> LUT = {{/* FFF: */ {NFF, FNF, FFN},
                                                  /* NFF: */ {FFF, FNF, FFN},
                                                  /* LFF: */ {FFF, LNF, LFN},
                                                  /* FNF: */ {NFF, FFF, FFN},
                                                  /* FLF: */ {NLF, FFF, FLN},
                                                  /* FFN: */ {NFF, FNF, FFF},
                                                  /* FFL: */ {NFL, FNL, FFF},
                                                  /* ---  */ {FFF, FFF, FFF},
                                                  /* LNF: */ {FFF, FFF, LFN},
                                                  /* LFN: */ {FFF, LNF, FFF},
                                                  /* NLF: */ {FFF, FFF, FLN},
                                                  /* FLN: */ {NLF, FFF, FFF},
                                                  /* NFL: */ {FFF, FNL, FFF},
                                                  /* FNL: */ {NFL, FFF, FFF}}};
      auto res = LUT[S][pos];
      assert(res != FFF);
      return res;
    };

    /// This is probably the method which is hardest to get right:
    /// If only F exist, just set it from F to N
    /// If N and F exist in parallel, we have to switch those two positions
    /// (practically discarding the "old" N)
    state_t S, NewS;
    do {
      S = state;
      NewS = static_cast<state_t>(getNewState(static_cast<STATE>(S)));
    } while (!state.compare_exchange_weak(S, NewS));
  }

  bool onBeforeStart() { return true; }

protected:
  void update_proto(std::function<void(return_t &)> updater) {
    for (auto &slot : slots) {
      updater(slot);
    }
  }

public:
  bool start() {
    if(!static_cast<provider_t &>(*this).onBeforeStart())
      return false;
    provider_thread = std::thread([this] {
      while (!stopped) {
        auto S = static_cast<STATE>(state.load());
        auto pos = getWritePosition(S);
        static_cast<provider_t &>(*this).produce(slots[pos]);
        commitUpdate(pos);
      }
    });
    return true;
  }
  void stop() {
    stopped = true;
    if (provider_thread.joinable())
      provider_thread.join();
  }
  friend self_t &operator>>(self_t &self, return_t &assigned) {
    STATE S = static_cast<STATE>(self.state.load());
    if (S ==
        FFF) // we don't *have to* update @p assigned if state is FFF
      return self;
    assert(S < LNF);
    S = self.acquire_read_lock();
    assigned = self.slots[self.getLockedPosition()];
    self.release_read_lock();
    return self;
  }
};