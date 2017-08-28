#include "ImageProvider.hpp"
#include <SDL.h>
#include <iostream>

ImageProvider::ImageProvider()
    : AsyncMostRecentDataStream<ImageProvider, ImageData>() {}

bool ImageProvider::onBeforeStart() {
  if (!webcam.open() || !webcam.getFrameSize(webcam_width, webcam_height)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam",
                             "Could not open webcam", NULL);
    return false;
  }

  update_proto([this](ImageData &b) {
    b.webcam_pixels.resize(size() * 3);
    std::fill(b.webcam_pixels.begin(), b.webcam_pixels.end(), 0.f);
  });
  return true;
}
void ImageProvider::onAfterStop() { webcam.close(); }

void ImageProvider::produce(ImageData &assigned) {
  if (!webcam.getFrame(assigned.webcam_pixels.data())) {
    std::cerr << "webcam lost, you need to restart the app\n";
  }
}
