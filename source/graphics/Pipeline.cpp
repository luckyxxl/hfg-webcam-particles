#include "main.hpp"

#include "Pipeline.hpp"

namespace graphics {

static GLuint createShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint infoLogLength;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
  if(infoLogLength > 0) {
    std::vector<char> log(infoLogLength);
    glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
    std::cout << "shader info log: \n" << log.data() << "\n";
  }

  GLint compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
  if(compileStatus == GL_FALSE) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not compile shader", "Could not compile shader", nullptr);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

static GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);

  glBindAttribLocation(program, 0, "v_position");
  glBindAttribLocation(program, 1, "v_rgb");
  glBindAttribLocation(program, 2, "v_hsv");
  glBindAttribLocation(program, 3, "v_localEffectStrength");

  glLinkProgram(program);

  GLint infoLogLength;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
  if(infoLogLength > 0) {
    std::vector<char> log(infoLogLength);
    glGetProgramInfoLog(program, log.size(), nullptr, log.data());
    std::cout << "program info log: \n" << log.data() << "\n";
  }

  GLint linkStatus;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if(linkStatus == GL_FALSE) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not link program", "Could not link program", nullptr);
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

bool Pipeline::create(const char *vertexShaderSource, const char *fragmentShaderSource) {
  vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
  fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  if(!vertexShader || !fragmentShader) return false;

  program = createProgram(vertexShader, fragmentShader);
  if(!program) return false;

  // This is the same for all pipelines for now... If required, set those in bind().
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  return true;
}

void Pipeline::destroy() {
  glDeleteProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Pipeline::bind() const {
  glUseProgram(program);
}

}
