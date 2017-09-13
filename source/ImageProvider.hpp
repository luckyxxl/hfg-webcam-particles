#pragma once

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

#include "ThreadSyncTripleBuffer.hpp"

class Resources;

struct ImageData {
  cv::Mat webcam_pixels;
  std::vector<cv::Rect> faces;

  ImageData() = default;
  ImageData(const ImageData &) = default;
  ImageData(ImageData &&) = default;
  ImageData &operator=(const ImageData &) = default;
  ImageData &operator=(ImageData &&) = default;

  void resize(size_t width, size_t height);
  bool empty() { return webcam_pixels.empty(); }
};

class ImageProvider {
public:
  bool create(Resources *resources);
  void destroy() {}

  bool start();
  void stop();

  size_t width() const { return webcam_size.width; }
  size_t height() const { return webcam_size.height; }
  size_t size() const { return width() * height(); }
  ImageData *consume() { return data.startCopyNew(); }
  void consumed() { data.finishCopy(); }

private:
  cv::VideoCapture capture;
  cv::Size webcam_size;
  ThreadSyncTripleBuffer<ImageData> data;
  cv::CascadeClassifier face_cascade;
  cv::CascadeClassifier eyes_cascade;

  std::atomic<bool> kill_threads{false};

  void webcamThreadFunc();
  std::thread webcam_thread;
};
