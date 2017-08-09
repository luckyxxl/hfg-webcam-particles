#include "main.hpp"

#include "Framebuffer.hpp"

namespace graphics {

void Framebuffer::create(uint32_t width, uint32_t height) {
  texture.create(width, height);

  glGenFramebuffers(1, &framebuffer);
  bind();
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.texture, 0);
}

void Framebuffer::destroy() {
  glDeleteFramebuffers(1, &framebuffer);
  texture.destroy();
}

void Framebuffer::resize(uint32_t width, uint32_t height) {
  texture.resize(width, height);
}

void Framebuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void Framebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace graphics
