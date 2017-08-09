#pragma once

namespace graphics {

class ScreenRectBuffer {
public:
  bool create();
  void destroy();

  void draw() const;

private:
  GLuint vertexBuffer = 0;
  GLuint vertexArray = 0;
};

} // namespace graphics
