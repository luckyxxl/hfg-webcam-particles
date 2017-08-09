#pragma once

namespace graphics {

class Pipeline {
public:
  bool create(const char *vertexShaderSource, const char *fragmentShaderSource, bool enableBlending);
  void destroy();

  void bind() const;

  GLint getUniformLocation(const char *uniformName) const {
    return glGetUniformLocation(program, uniformName);
  }

private:
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;
  GLuint program = 0;

  bool enableBlending = false;
};

} // namespace graphics
