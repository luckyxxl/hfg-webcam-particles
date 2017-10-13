#include "main.hpp"

#include "ImageProvider.hpp"
#include "Resources.hpp"

static cv::Mat debug_image;

bool ImageProvider::create(Resources *resources) {
  debug_image = cv::imread("Rayman2_1920x1080.jpg");
  webcam_size = debug_image.size();
  return true;
}

bool ImageProvider::start() {
  for(size_t i=0u; i<data.size; ++i) {
    data.getBuffer(i).webcam_pixels = cv::Mat::zeros(webcam_size.height, webcam_size.width, CV_8UC3);
  }

  webcam_thread = std::thread([this] { this->webcamThreadFunc(); });

  return true;
}
void ImageProvider::stop() {
  if(webcam_thread.joinable()) {
    kill_threads = true;
    webcam_thread.join();
  }
}

void ImageProvider::webcamThreadFunc() {
  while(!kill_threads) {
    auto assigned = data.startWrite();

    auto &frame = assigned->webcam_pixels;
    auto &faces = assigned->faces;

    frame = debug_image;

    faces.clear();
    faces.emplace_back(1142, 36, 400, 400);

#if 0
    for(const auto &f : faces) {
      cv::rectangle(frame, f, cv::Scalar(0, 0, 255));
    }
#endif

    data.finishWrite();

    SDL_Delay(10u);
  }
}
