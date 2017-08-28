#pragma once

#include "AsyncMostRecentDataStream.hpp"
#include "Webcam.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <future>
#include <cstdint>

struct ImageData {
  ImageData() = default;
  ImageData(const ImageData &) = default;
  ImageData(ImageData &&) = default;
  ImageData &operator=(const ImageData &) = default;
  ImageData &operator=(ImageData &&) = default;
  std::vector<float> webcam_pixels;
  bool empty() { return webcam_pixels.empty(); }
};

class ImageProvider : public AsyncMostRecentDataStream<ImageProvider, ImageData> {
  friend class AsyncMostRecentDataStream<ImageProvider, ImageData>;
public:
  ImageProvider();

  size_t width() const { return webcam_width; }
  size_t height() const { return webcam_height; }
  size_t size() const { return width() * height(); }

private:
  bool onBeforeStart();
  void onAfterStop();
  void produce(ImageData &assigned);
private:
  Webcam webcam;
  uint32_t webcam_width, webcam_height;
};
