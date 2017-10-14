#pragma once

#include "graphics/Framebuffer.hpp"
#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/Pipeline.hpp"

class OverlayCompose {
public:
  void create(const graphics::ScreenRectBuffer *rectangle);
  void destroy();

  void draw(graphics::Texture &input, graphics::Texture &overlay, float overlayVisibility, graphics::Framebuffer &output);

private:
  const graphics::ScreenRectBuffer *rectangle;
  graphics::Pipeline pipeline;
  GLint pipeline_webcam_location;
  GLint pipeline_overlay_location;
  GLint pipeline_overlayVisibility_location;
};
