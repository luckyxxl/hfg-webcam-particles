#pragma once

#include "ThreadSyncTripleBuffer.hpp"
#include "Webcam.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <future>
#include <cstdint>

struct ImageData {
  std::vector<float> webcam_pixels;
};

class ImageProvider {
public:
  bool start();
  std::future<void> stop();
  void destroy();
  ImageProvider();

  size_t width() const { return webcam_width; }
  size_t height() const { return webcam_height; }
  size_t size() const { return width() * height(); }
  ImageData *consume() { return data.startCopyNew(); }
  void consumed() { data.finishCopy(); }
private:
  Webcam webcam;
  uint32_t webcam_width, webcam_height;
  ThreadSyncTripleBuffer<ImageData> data;

  std::atomic<bool> kill_threads{false};

  void webcamThreadFunc();
  std::thread webcam_thread;
};
