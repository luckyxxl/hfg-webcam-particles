#pragma once

class Webcam {
  public:
  Webcam();
  ~Webcam();

  void getFrameSize(uint32_t &width, uint32_t &height);
  void getFrame(float *frame);

  private:
  cv::VideoCapture capture;
};
