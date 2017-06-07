#include "main.hpp"

#include "Webcam.hpp"

Webcam::Webcam() {
  capture.open(0);
  if(!capture.isOpened()) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam", "Could not open webcam", NULL);
    return;
  }
}

Webcam::~Webcam() {
  capture.release();
}

void Webcam::getFrameSize(uint32_t &width, uint32_t &height) {
  width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
}

void Webcam::getFrame(float *frame) {
  if(!capture.isOpened()) {
    uint32_t w, h;
    getFrameSize(w, h);
    std::fill(frame, frame + w * h * 3, 0.f);
    return;
  }

  cv::Mat mat;
  capture.read(mat);

#ifndef NDEBUG
  {
    uint32_t w, h;
    getFrameSize(w, h);
    assert(static_cast<int>(w) == mat.cols && static_cast<int>(h) == mat.rows);
  }
#endif

  cv::Mat result;
  cv::cvtColor(mat, result, CV_BGR2RGB);
  cv::flip(result, result, 0);
  result.convertTo(result, CV_32F, 1 / 255.);

  memcpy(frame, result.data, result.cols * result.rows * 3 * sizeof(float));
}
