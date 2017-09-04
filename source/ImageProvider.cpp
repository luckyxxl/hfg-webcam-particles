#include "ImageProvider.hpp"
#include "Resources.hpp"
#include <SDL.h>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

static const char *face_cascade_xml = "haarcascade_frontalface_alt.xml";
static const char *eyes_cascade_xml = "haarcascade_eye_tree_eyeglasses.xml";

void ImageData::resize(size_t width, size_t height) {
  normalized_pixels = cv::Mat::zeros(height, width, CV_32FC3);
}

ImageProvider::ImageProvider()
    : AsyncMostRecentDataStream<ImageProvider, ImageData>() {}

static cv::Size getFrameSize(cv::VideoCapture &capture) {
#if CV_VERSION_EPOCH < 3
  int w = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  int h = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
#else
  int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);
  int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
#endif
  return {w, h};
}

bool ImageProvider::onBeforeStart() {
  capture.open(0);
  if (capture.isOpened()) {
    webcam_size = getFrameSize(capture);
    if (webcam_size.width > 0 && webcam_size.height > 0) {
      update_proto([this](ImageData &b) {
        b.resize(webcam_size.width, webcam_size.height);
      });
      face_cascade.load(resources->resolve(face_cascade_xml));
      return true;
    }
  }
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam",
                           "Could not open webcam", NULL);
  return false;
}
void ImageProvider::onAfterStop() { capture.release(); }

void ImageProvider::produce(ImageData &assigned) {
  auto &frame = assigned.webcam_pixels;
  auto &normalized = assigned.normalized_pixels;
  auto &faces = assigned.faces;

  if (!capture.isOpened()) {
    return;
  }
  if (!capture.read(frame)) {
    return;
  }
#ifndef NDEBUG
  {
    auto S = getFrameSize(capture);
    assert(static_cast<int>(S.width) == frame.cols &&
           static_cast<int>(S.height) == frame.rows);
  }
#endif

  frame.convertTo(normalized, CV_32F, 1 / 255.);
  cv::cvtColor(normalized, normalized, CV_BGR2RGB);
  cv::flip(normalized, normalized, 0);

  cv::Mat frame_gray;

  cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
  cv::equalizeHist(frame_gray, frame_gray);
  face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2,
                                0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
  if (faces.size() > 0) {
    std::cout << "Detected faces: " << faces.size() << "\n";
  }
}
