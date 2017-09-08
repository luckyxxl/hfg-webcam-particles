#pragma once

#include "Texture.hpp"

namespace graphics {

class Framebuffer {
public:
  void create(uint32_t width, uint32_t height);
  void destroy();

  uint32_t getWidth() const { return texture.getWidth(); }
  uint32_t getHeight() const { return texture.getHeight(); }

  void resize(uint32_t width, uint32_t height);

  void bind();
  static void unbind();

  void clear() { bind(); glClear(GL_COLOR_BUFFER_BIT); }

  Texture &getTexture() { return texture; }

private:
  Texture texture;
  GLuint framebuffer = 0;
};

} // namespace graphics
