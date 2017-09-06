#pragma once

#include "graphics/Texture.hpp"
#include "graphics/ParticleBuffer.hpp"

class ParticleTextureToBuffer {
public:
  void create();
  void destroy();

  void render(uint32_t width, uint32_t height, graphics::Texture &particleTexture,
    graphics::Texture &backgroundTexture, graphics::ParticleBuffer &particleBuffer);

private:
  GLuint vao;
  GLuint program;

  GLint width_location;
  GLint height_location;
  GLint particleTexture_location;
  GLint backgroundTexture_location;
};
