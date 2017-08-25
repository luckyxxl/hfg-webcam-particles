#pragma once

#include <atomic>
#include <array>
#include <cstdint>
#include <cassert>
#include <iterator>

template <class T> class ThreadSyncTripleBuffer {
  using dataId_t = uint32_t;
  struct Buffer {
    dataId_t dataId;
    T data;
  };
public:
  static constexpr auto size = 3;
  using buffers_t = std::array<Buffer, size>;

  ThreadSyncTripleBuffer()
      : writeId(0), previousCopyDataId(writeId), writeBuffer(0), readBuffer(1),
        copyBuffer(2) {
    for (auto &b : buffers) {
      b.dataId = writeId;
    }
  }

  T &startWrite() {
    buffers[writeBuffer].dataId = ++writeId;
    return buffers[writeBuffer].data;
  }

  void finishWrite() {
    uint32_t newWrite = readBuffer;
    while (!readBuffer.compare_exchange_weak(newWrite, writeBuffer))
      ;
    writeBuffer = newWrite;
  }

  T *startCopyNew() {
    // No race condition here, only check whether this was changed (can only be
    // by finishWrite). It's no error if even more recent data is inserted in
    // between.
    if (buffers[readBuffer].dataId != previousCopyDataId) {
      previousCopyDataId = buffers[copyBuffer].dataId;

      uint32_t newCopy = readBuffer;
      while (!readBuffer.compare_exchange_weak(newCopy, copyBuffer))
        ;
      copyBuffer = newCopy;

      return &buffers[copyBuffer].data;
    }
    return nullptr;
  }

  void finishCopy() {}

  // not threadsafe!
  T &getBuffer(int i) {
    assert(i >= 0 && i < size);
    return buffers[i].data;
  }

  class iterator {
    friend class ThreadSyncTripleBuffer<T>;
    using it_t = typename ThreadSyncTripleBuffer<T>::buffers_t::iterator;
    it_t It;
    iterator(it_t It) : It(It) {}
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using pointer = T*;
    using iterator_category = std::forward_iterator_tag;
    iterator() = default;
    iterator(const iterator &) = default;
    iterator(iterator &&) = default;
    iterator &operator=(const iterator &) = default;
    iterator &operator=(iterator &&) = default;
    T &operator*() { return It->data; }
    const T &operator*() const { return It->data; }
    T &operator->() { return *this; }
    const T &operator->() const { return *this; }
    bool operator==(const iterator &o) const { return It == o.It; }
    bool operator!=(const iterator &o) const { return It != o.It; }
    iterator &operator++() { ++It; return *this; }
    iterator operator++(int) { auto copy = *this; ++(*this); return copy; }
  };
  iterator begin() {
    return buffers.begin();
  }
  iterator end() {
    return buffers.end();
  }

private:
  dataId_t writeId;

  dataId_t previousCopyDataId;

  uint32_t writeBuffer;
  std::atomic<uint32_t> readBuffer;
  uint32_t copyBuffer;

  buffers_t buffers;
};
