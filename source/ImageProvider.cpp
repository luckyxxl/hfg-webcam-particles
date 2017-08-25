#include "ImageProvider.hpp"
#include <iostream>
#include <SDL.h>

ImageProvider::ImageProvider() {}

bool ImageProvider::start() {
  if (!webcam.open() || !webcam.getFrameSize(webcam_width, webcam_height)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam",
                             "Could not open webcam", NULL);
    return false;
  }

  for (auto &b : data) {
    b.webcam_pixels.resize(size() * 3);
    std::fill(b.webcam_pixels.begin(), b.webcam_pixels.end(), 0.f);
  }

  webcam_thread = std::thread([this] { this->webcamThreadFunc(); });
  return true;
}
std::future<void> ImageProvider::stop() {
  kill_threads = true;
  return std::async(std::launch::deferred, [this] {
    if (webcam_thread.joinable())
      webcam_thread.join();
  });
}

void ImageProvider::destroy() {
  webcam.close();
}

void ImageProvider::webcamThreadFunc() {
  while (!kill_threads) {
    auto &frame = data.startWrite();
    if (!webcam.getFrame(frame.webcam_pixels.data())) {
      std::cerr << "webcam lost, you need to restart the app\n";
      break;
    }
    data.finishWrite();
  }
}
