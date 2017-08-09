#include "main.hpp"

#include "ScreenRectBuffer.hpp"

namespace graphics {

bool ScreenRectBuffer::create() {
  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

  const GLfloat data[] = {
    -1, -1,
    -1, 1,
    1, -1,
    1, 1,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0,
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  return true;
}

void ScreenRectBuffer::destroy() {
  glDeleteVertexArrays(1, &vertexArray);
  glDeleteBuffers(1, &vertexBuffer);
}

void ScreenRectBuffer::draw() const {
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // namespace graphics
