#include "main.hpp"

#include "DebugImageProvider.hpp"

bool DebugImageProvider::create(Resources *resources) {
  imageData.webcam_pixels = cv::imread(resources->resolve("Rayman2_1920x1080.jpg"));
  imageData.faces.emplace_back(1140, 129, 1461-1140, 454-129);
  return true;
}
