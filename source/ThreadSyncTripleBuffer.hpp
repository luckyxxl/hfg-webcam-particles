template <class T> class ThreadSyncTripleBuffer {
public:
  static constexpr auto size = 3;
  using dataId_t = uint32_t;

  ThreadSyncTripleBuffer()
      : writeId(0), previousCopyDataId(writeId), writeBuffer(0), readBuffer(1),
        copyBuffer(2) {
    for (auto i = 0; i < size; ++i) {
      buffers[i].dataId = writeId;
    }
  }

  T *startWrite() {
    buffers[writeBuffer].dataId = ++writeId;
    return &buffers[writeBuffer].data;
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

private:
  dataId_t writeId;

  dataId_t previousCopyDataId;

  uint32_t writeBuffer;
  std::atomic<uint32_t> readBuffer;
  uint32_t copyBuffer;

  struct Buffer {
    dataId_t dataId;
    T data;
  };

  Buffer buffers[size];
};
