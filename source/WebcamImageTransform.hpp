#pragma once

#include "graphics/Framebuffer.hpp"
#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/Pipeline.hpp"

class WebcamImageTransform {
public:
  void create(const graphics::ScreenRectBuffer *rectangle,
      uint32_t input_width, uint32_t input_height,
      uint32_t output_width, uint32_t output_height);
  void destroy();

  const glm::mat3 &getTransform() const { return transform; }
  const glm::mat3 &getInverseTransform() const { return inverseTransform; }

  void draw(graphics::Texture &input, graphics::Framebuffer &output);

#if WITH_EDIT_TOOLS
  float *editGetBrightnessMulP() { return &brightnessMul; }
  float *editGetBrightnessAddP() { return &brightnessAdd; }
  float *editGetSaturationP() { return &saturation; }
#endif

private:
  const graphics::ScreenRectBuffer *rectangle;
  graphics::Pipeline pipeline;
  GLint pipeline_transform_location;
  GLint pipeline_source_location;
  GLint pipeline_imageParameters_location;

  uint32_t input_width, input_height;
  uint32_t output_width, output_height;

  float brightnessMul = 1.0f, brightnessAdd = 0.f, saturation = 1.5f;

  glm::mat3 transform, inverseTransform;
};
