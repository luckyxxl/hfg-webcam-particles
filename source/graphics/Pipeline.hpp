#pragma once

namespace graphics {

class Pipeline {
public:
  enum class BlendMode {
    None,
    Normal,
    Addition,
  };

  bool create(const char *vertexShaderSource, const char *fragmentShaderSource, BlendMode blendMode);
  void destroy();

  void bind() const;

  GLint getUniformLocation(const char *uniformName) const {
    return glGetUniformLocation(program, uniformName);
  }

private:
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;
  GLuint program = 0;

  BlendMode blendMode = BlendMode::None;
};

} // namespace graphics
