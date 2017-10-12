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

  void draw(graphics::Texture &input, graphics::Framebuffer &output);

private:
  const graphics::ScreenRectBuffer *rectangle;
  graphics::Pipeline pipeline;
  GLint pipeline_transform_location;
  GLint pipeline_source_location;

  uint32_t input_width, input_height;
  uint32_t output_width, output_height;

  glm::mat3 transform;
};
