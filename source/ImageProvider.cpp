#include "main.hpp"

#include "ImageProvider.hpp"
#include "Resources.hpp"

static const char *face_cascade_xml = "haarcascade_frontalface_alt.xml";
static const char *eyes_cascade_xml = "haarcascade_eye_tree_eyeglasses.xml";

static cv::Size getFrameSize(cv::VideoCapture &capture) {
#if defined(CV_VERSION_EPOCH)
  int w = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  int h = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
#else
  int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);
  int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
#endif
  return {w, h};
}

bool ImageProvider::create(Resources *resources) {
  if(!face_cascade.load(resources->resolve(face_cascade_xml))) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could load CascadeClassifier XML",
                           "Could load face CascadeClassifier XML", NULL);
    return false;
  }
  (void)eyes_cascade_xml; // fix unused variable warning
  return true;
}

bool ImageProvider::start() {
  capture.open(0);
  if (capture.isOpened()) {
#if defined(CV_VERSION_EPOCH) || CV_VERSION_MAJOR < 3
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(CV_CAP_PROP_FPS, 30);
#else
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(cv::CAP_PROP_FPS, 30);
#endif
    webcam_size = getFrameSize(capture);
    std::cout << "webcam resolution: " << webcam_size.width << "x" << webcam_size.height << "\n";
    if (webcam_size.width > 0 && webcam_size.height > 0) {
      for(size_t i=0u; i<data.size; ++i) {
        data.getBuffer(i).webcam_pixels = cv::Mat::zeros(webcam_size.height, webcam_size.width, CV_8UC3);
      }

      webcam_thread = std::thread([this] { this->webcamThreadFunc(); });
      face_detection_thread = std::thread([this] { this->faceDetectionThreadFunc(); });

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
    face_detection_thread.join();
  }

  capture.release();
}

void ImageProvider::webcamThreadFunc() {
  while(!kill_threads) {
    auto assigned = data.startWrite();
    auto face_detection_frame = face_detection_data.startWrite();

    auto &frame = assigned->webcam_pixels;
    auto &faces_size = assigned->detected_faces;

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

    faces_size = detected_faces.load(std::memory_order_relaxed);
    *face_detection_frame = frame;

    face_detection_data.finishWrite();
    data.finishWrite();
  }
}

void ImageProvider::faceDetectionThreadFunc() {

  cv::Mat frame_gray;
  std::vector<cv::Rect> faces;

  while(!kill_threads) {
    auto frame_ptr = face_detection_data.startCopyNew();
    if(!frame_ptr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    auto frame = *frame_ptr;

#if defined(CV_VERSION_EPOCH) || CV_VERSION_MAJOR < 3
    cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
#else
    cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
#endif
    cv::equalizeHist(frame_gray, frame_gray);
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 4,
#if defined(CV_VERSION_EPOCH) || CV_VERSION_MAJOR < 3
                                  0 | CV_HAAR_SCALE_IMAGE,
#else
                                  0 | cv::CASCADE_SCALE_IMAGE,
#endif
				  cv::Size(30, 30));

#if 0
    for(const auto &f : faces) {
      cv::rectangle(frame, f, cv::Scalar(0, 0, 255));
    }
#endif

    detected_faces.store(faces.size(), std::memory_order_relaxed);
  }
}
