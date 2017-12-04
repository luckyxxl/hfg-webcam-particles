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
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(CV_CAP_PROP_FPS, 30);
    webcam_size = getFrameSize(capture);
    std::cout << "webcam resolution: " << webcam_size.width << "x" << webcam_size.height << "\n";
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

    cv::Mat frame_gray_rot;
#if CV_VERSION_EPOCH < 3 //TODO: not really, cv::rotate came out in 3.3.1 i think
    cv::transpose(frame_gray, frame_gray_rot);
    cv::flip(frame_gray_rot, frame_gray_rot, 1);
#else
    cv::rotate(frame_gray, frame_gray_rot, cv::ROTATE_90_CLOCKWISE);
#endif

    face_cascade.detectMultiScale(frame_gray_rot, faces, 1.1, 4,
                                  0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));

#if 0
    std::cout << "Faces: " << faces.size() << "\n";
    for(const auto &f : faces) {
      cv::Rect f_rot(f.y, frame.rows - f.width - f.x, f.height, f.width);
      cv::rectangle(frame, f_rot, cv::Scalar(0, 0, 255));
    }
#endif

    data.finishWrite();
  }
}
