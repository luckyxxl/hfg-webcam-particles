#pragma once

namespace graphics {

class Texture {
public:
  void create(uint32_t width, uint32_t height);
  void destroy();

  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }

  void resize(uint32_t width, uint32_t height);

  void setImage(uint32_t width, uint32_t height, const uint8_t *pixels);

  void bind(uint32_t unit);
  static void unbind(uint32_t unit);

  // don't use this function :)
  void dbgSaveToFile(const char *filename);

private:
  GLuint texture = 0;
  uint32_t width = 0, height = 0;
  friend class Framebuffer;
};

} // namespace graphics
