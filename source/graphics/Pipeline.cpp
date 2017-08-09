#include "main.hpp"

#include "Pipeline.hpp"

namespace graphics {

static GLuint createShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint infoLogLength;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    std::vector<char> log(infoLogLength);
    glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
    std::cout << "shader info log: \n" << log.data() << "\n";
  }

  GLint compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if (compileStatus == GL_FALSE) {
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

  glLinkProgram(program);

  GLint infoLogLength;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    std::vector<char> log(infoLogLength);
    glGetProgramInfoLog(program, log.size(), nullptr, log.data());
    std::cout << "program info log: \n" << log.data() << "\n";
  }

  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not link program",
                             "Could not link program", nullptr);
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

bool Pipeline::create(const char *vertexShaderSource,
                      const char *fragmentShaderSource,
                      bool enableBlending) {
  vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
  fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  if (!vertexShader || !fragmentShader)
    return false;

  program = createProgram(vertexShader, fragmentShader);
  if (!program)
    return false;

  this->enableBlending = enableBlending;

  // This is the same for all pipelines for now... If required, set those in
  // bind().
  glEnable(GL_PROGRAM_POINT_SIZE);
  glBlendFunc(GL_ONE, GL_ONE);

  return true;
}

void Pipeline::destroy() {
  glDeleteProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Pipeline::bind() const {
  if(enableBlending) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  glUseProgram(program);
}

} // namespace graphics
