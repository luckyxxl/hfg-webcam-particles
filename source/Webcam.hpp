#pragma once

class Webcam {
  public:
  bool open();
  void close();

  bool getFrameSize(uint32_t &width, uint32_t &height);
  bool getFrame(float *frame);

  private:
  cv::VideoCapture capture;
};
