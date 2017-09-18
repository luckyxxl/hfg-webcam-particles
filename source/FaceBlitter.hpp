#pragma once

#include "graphics/Framebuffer.hpp"
#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/Pipeline.hpp"

class FaceBlitter {
public:
  void create(const graphics::ScreenRectBuffer *rectangle);
  void destroy();

  void blit(graphics::Texture &source, glm::vec2 sourceMin, glm::vec2 sourceMax,
    graphics::Framebuffer &target, glm::vec2 targetMin, glm::vec2 targetMax,
    graphics::Texture &background);

private:
  const graphics::ScreenRectBuffer *rectangle;
  graphics::Pipeline pipeline;
  GLint sourceCenter_location;
  GLint sourceExtend_location;
  GLint targetCenter_location;
  GLint targetExtend_location;
  GLint source_location;
  GLint background_location;
};
