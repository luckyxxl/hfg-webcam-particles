#include "main.hpp"

#include "Texture.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace graphics {

void Texture::create(uint32_t width, uint32_t height) {
  glGenTextures(1, &texture);
  resize(width, height);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::destroy() {
  glDeleteTextures(1, &texture);
}

void Texture::resize(uint32_t width, uint32_t height) {
  bind(0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  this->width = width;
  this->height = height;
}

void Texture::setImage(uint32_t width, uint32_t height, const uint8_t *pixels) {
  assert(width == this->width && height == this->height);
  bind(0);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void Texture::bind(uint32_t unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::unbind(uint32_t unit) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::dbgSaveToFile(const char *filename) {
  bind(0);

  auto pixels = std::vector<uint8_t>(width * height * 3);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

  stbi_write_bmp(filename, width, height, 3, pixels.data());
}

} // namespace graphics
