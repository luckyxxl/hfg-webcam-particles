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
  std::atomic_int_fast8_t state;
  using state_t = std::remove_reference_t<decltype(state.load())>;

  /// Slot roles encoded as New (N), Free (F), Locked (L)
  // Invariants:
  // * locked versions are always +1
  // * the position of the lock is denoted by the second and third bit
  // interpreted as unary
  // * if 4th bit is set, new and locked exist in parallel
  // * there is always at least one F slot available
  static inline constexpr state_t FFF = 0;
  static inline constexpr state_t NFF = 1; // +1 locks
  static inline constexpr state_t LFF = 2;
  static inline constexpr state_t FNF = 3; // +1 locks
  static inline constexpr state_t FLF = 4;
  static inline constexpr state_t FFN = 5; // +1 locks
  static inline constexpr state_t FFL = 6;

  static inline constexpr state_t LNF = 8;
  static inline constexpr state_t LFN = 9;
  static inline constexpr state_t NLF = 10;
  static inline constexpr state_t FLN = 11;
  static inline constexpr state_t NFL = 12;
  static inline constexpr state_t FNL = 13;

  static inline constexpr state_t ERR = 66;

  state_t acquire_read_lock() {
    assert(state != FFF);
    assert(state < FFL);
    assert(state % 2 == 1);
    return ++state;
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
  void commitUpdate(int pos) {
    auto getNewState = [pos](state_t S) {
      std::array<std::array<state_t, 3>, 14> LUT = {
          {/* FFF: */ {NFF, FNF, FFN},
           /* NFF: */ {ERR, FNF, FFN},
           /* LFF: */ {ERR, LNF, LFN},
           /* FNF: */ {NFF, ERR, FFN},
           /* FLF: */ {NLF, ERR, FLN},
           /* FFN: */ {NFF, FNF, ERR},
           /* FFL: */ {NFL, FNL, ERR},
           /* ---  */ {ERR, ERR, ERR},
           /* LNF: */ {ERR, ERR, LFN},
           /* LFN: */ {ERR, LNF, ERR},
           /* NLF: */ {ERR, ERR, FLN},
           /* FLN: */ {NLF, ERR, ERR},
           /* NFL: */ {ERR, FNL, ERR},
           /* FNL: */ {NFL, ERR, ERR}}};
      auto res = LUT[S][pos];
      assert(res != ERR);
      return res;
    };

    /// This is probably the method which is hardest to get right:
    /// If only F exist, just set it from F to N
    /// If N and F exist in parallel, we have to switch those two positions
    /// (practically discarding the "old" N)
    state_t S, NewS;
    do {
      S = state;
      NewS = getNewState(S);
    } while (!state.compare_exchange_weak(S, NewS));
  }

  static int getLockedPosition(state_t state) {
    assert(state != FFF && state != ERR);
    assert(state < LNF); // This makes things much easier and the only use-case
                         // of this method actually allows us to ignore the
                         // other cases
    assert(state % 2 == 0);
    auto bits = (state >> 1) & 0B11;
    assert(bits);
    return bits - 1;
  }
  static int getWritePosition(state_t S) {
    if (S < 5)
      return 2;
    if (S < 8)
      return 0;
    switch (S) {
    case FLN:
    case FNL:
      return 0;
    case NFL:
    case LFN:
      return 1;
    case NLF:
    case LNF:
      return 2;
    default:
      assert(false);
    }
    return 0;
  }

protected:
  bool onBeforeStart() { return true; }
  void update_proto(std::function<void(return_t &)> updater) {
    for (auto &slot : slots) {
      updater(slot);
    }
  }

public:
  bool start() {
    if (!static_cast<provider_t &>(*this).onBeforeStart())
      return false;
    provider_thread = std::thread([this] {
      while (!stopped) {
        auto S = state.load();
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
    state_t S = self.state.load();
    if (S == FFF) // we don't *have to* update @p assigned if state is FFF
      return self;
    assert(S < LNF);
    S = self.acquire_read_lock();
    assigned = self.slots[getLockedPosition(S)];
    self.release_read_lock();
    return self;
  }
};
