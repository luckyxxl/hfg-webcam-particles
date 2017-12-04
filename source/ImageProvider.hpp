#pragma once

#include "ThreadSyncTripleBuffer.hpp"

struct ImageData {
  cv::Mat webcam_pixels;
  size_t detected_faces;
};

class Resources;

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

private:
  cv::VideoCapture capture;
  cv::Size webcam_size;
  ThreadSyncTripleBuffer<ImageData> data;
  cv::CascadeClassifier face_cascade;
  //cv::CascadeClassifier eyes_cascade;

  std::atomic<bool> kill_threads{false};

  void webcamThreadFunc();
  std::thread webcam_thread;

  ThreadSyncTripleBuffer<cv::Mat> face_detection_data;
  std::atomic<size_t> detected_faces{0u};

  void faceDetectionThreadFunc();
  std::thread face_detection_thread;
};
