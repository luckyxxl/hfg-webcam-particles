#include "main.hpp"

#include "Texture.hpp"

namespace graphics {

void Texture::create(uint32_t width, uint32_t height) {
  glGenTextures(1, &texture);
  resize(width, height);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::destroy() {
  glDeleteTextures(1, &texture);
}

void Texture::resize(uint32_t width, uint32_t height) {
  bind(0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
}

void Texture::bind(uint32_t unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, texture);
}


void Texture::unbind(uint32_t unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace graphics
