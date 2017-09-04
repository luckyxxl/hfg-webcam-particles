#pragma once

#include "AsyncMostRecentDataStream.hpp"
#include <atomic>
#include <cstdint>
#include <future>
#include <opencv2/objdetect/objdetect.hpp>
#if CV_VERSION_EPOCH < 3
#include <opencv2/highgui/highgui.hpp>
#else
#include <opencv2/videoio.hpp>
#endif
#include <thread>
#include <vector>

class Resources;

struct ImageData {
  cv::Mat webcam_pixels;
  cv::Mat normalized_pixels;
  std::vector<cv::Rect> faces;

  ImageData() = default;
  ImageData(const ImageData &) = default;
  ImageData(ImageData &&) = default;
  ImageData &operator=(const ImageData &) = default;
  ImageData &operator=(ImageData &&) = default;

  void resize(size_t width, size_t height);
  float *data() { return normalized_pixels.ptr<float>(); }
  bool empty() { return normalized_pixels.empty(); }
};

class ImageProvider
    : public AsyncMostRecentDataStream<ImageProvider, ImageData> {
  friend class AsyncMostRecentDataStream<ImageProvider, ImageData>;

public:
  ImageProvider();
  bool create(Resources &resources) {
    this->resources = &resources;
    return true;
  }

  size_t width() const { return webcam_size.width; }
  size_t height() const { return webcam_size.height; }
  size_t size() const { return width() * height(); }

private:
  bool onBeforeStart();
  void onAfterStop();
  void produce(ImageData &assigned);

private:
  Resources *resources;
  cv::VideoCapture capture;
  cv::Size webcam_size;
  cv::CascadeClassifier face_cascade;
  cv::CascadeClassifier eyes_cascade;
};
