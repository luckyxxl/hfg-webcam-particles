#include "main.hpp"

#include "Pipeline.hpp"
#include "Util.hpp"

namespace graphics {

static GLuint createShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);

  if(!Util::compileShader(shader)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not compile shader",
                             "Could not compile shader", nullptr);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

static GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);

  // keep in sync with ParticleRenderer
  glBindAttribLocation(program, 0, "position");
  glBindAttribLocation(program, 1, "rgb");
  glBindAttribLocation(program, 2, "hsv");
  glBindAttribLocation(program, 3, "foregroundMask");

  if(!Util::linkProgram(program)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not link program",
                             "Could not link program", nullptr);
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

bool Pipeline::create(const char *vertexShaderSource,
                      const char *fragmentShaderSource,
                      BlendMode blendMode) {
  vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
  fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  if (!vertexShader || !fragmentShader)
    return false;

  program = createProgram(vertexShader, fragmentShader);
  if (!program)
    return false;

  this->blendMode = blendMode;

  // This is the same for all pipelines for now... If required, set those in
  // bind().
  glEnable(GL_PROGRAM_POINT_SIZE);

  return true;
}

void Pipeline::destroy() {
  glDeleteProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Pipeline::bind() const {
  if(blendMode == BlendMode::None) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);

    switch(blendMode) {
      case BlendMode::None:
      break;
      case BlendMode::Normal:
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
      case BlendMode::Addition:
      glBlendFunc(GL_ONE, GL_ONE);
      break;
    }
  }

  glUseProgram(program);
}

} // namespace graphics
