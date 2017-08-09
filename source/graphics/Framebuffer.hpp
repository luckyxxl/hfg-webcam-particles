#pragma once

#include "Texture.hpp"

namespace graphics {

class Framebuffer {
public:
  void create(uint32_t width, uint32_t height);
  void destroy();

  void resize(uint32_t width, uint32_t height);

  void bind();
  static void unbind();

  Texture &getTexture() { return texture; }

private:
  Texture texture;
  GLuint framebuffer = 0;
};

} // namespace graphics
