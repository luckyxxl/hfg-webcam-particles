#include "main.hpp"

#include "ImageProvider.hpp"
#include "Resources.hpp"

static const char *face_cascade_xml = "haarcascade_frontalface_alt.xml";
static const char *eyes_cascade_xml = "haarcascade_eye_tree_eyeglasses.xml";

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

bool ImageProvider::create(Resources *resources) {
  face_cascade.load(resources->resolve(face_cascade_xml));
  (void)eyes_cascade_xml; // fix unused variable warning
  return true;
}

bool ImageProvider::start() {
  capture.open(0);
  if (capture.isOpened()) {
    webcam_size = getFrameSize(capture);
    if (webcam_size.width > 0 && webcam_size.height > 0) {
      for(size_t i=0u; i<data.size; ++i) {
        data.getBuffer(i).webcam_pixels = cv::Mat::zeros(webcam_size.height, webcam_size.width, CV_8UC3);
      }

      webcam_thread = std::thread([this] { this->webcamThreadFunc(); });

      return true;
    }
  }
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam",
                           "Could not open webcam", NULL);
  return false;
}
void ImageProvider::stop() {
  if(webcam_thread.joinable()) {
    kill_threads = true;
    webcam_thread.join();
  }

  capture.release();
}

void ImageProvider::webcamThreadFunc() {
  while(!kill_threads) {
    auto assigned = data.startWrite();

    auto &frame = assigned->webcam_pixels;
    auto &faces = assigned->faces;

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

    cv::Mat frame_gray;

    cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
    cv::equalizeHist(frame_gray, frame_gray);
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2,
                                  0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));

#if 0
    for(const auto &f : faces) {
      cv::rectangle(frame, f, cv::Scalar(0, 0, 255));
    }
#endif

    data.finishWrite();
  }
}
