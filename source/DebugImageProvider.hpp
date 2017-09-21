#pragma once

#include "ImageProvider.hpp"

class DebugImageProvider {
public:
  bool create(Resources *resources);
  void destroy() {}

  bool start() { return true; }
  void stop() {}

  size_t width() const { return imageData.webcam_pixels.cols; }
  size_t height() const { return imageData.webcam_pixels.rows; }
  size_t size() const { return width() * height(); }
  ImageData *consume() { return &imageData; }

private:
  ImageData imageData;
};
