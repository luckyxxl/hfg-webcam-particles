#pragma once

#include "graphics/Framebuffer.hpp"
#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/Pipeline.hpp"

class FinalComposite {
public:
  void create(const graphics::ScreenRectBuffer *rectangle);
  void destroy();

  void draw(graphics::Texture &source, graphics::Texture &overlay, float overlayVisibility, uint32_t screen_width, uint32_t screen_height);

private:
  const graphics::ScreenRectBuffer *rectangle;
  graphics::Pipeline pipeline;
  GLint pipeline_source_location;
  GLint pipeline_overlay_location;
  GLint pipeline_overlayVisibility_location;
};
