#include "main.hpp"

#include "Webcam.hpp"

bool Webcam::open() {
  capture.open(0);
  if (!capture.isOpened()) {
    return false;
  }
  return true;
}

void Webcam::close() { capture.release(); }

bool Webcam::getFrameSize(uint32_t &width, uint32_t &height) {
  width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
  return width != 0 && height != 0;
}

bool Webcam::getFrame(float *frame) {
  if (!capture.isOpened()) {
    return false;
  }

  cv::Mat image;
  if (!capture.read(image)) {
    return false;
  }

#ifndef NDEBUG
  {
    uint32_t w, h;
    getFrameSize(w, h);
    assert(static_cast<int>(w) == image.cols &&
           static_cast<int>(h) == image.rows);
  }
#endif

  cv::cvtColor(image, image, CV_BGR2RGB);
  cv::flip(image, image, 0);

  cv::Mat result(image.rows, image.cols, CV_32FC3, frame);
  image.convertTo(result, CV_32F, 1 / 255.);

  return true;
}
