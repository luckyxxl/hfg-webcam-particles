#include "main.hpp"

#include "Util.hpp"

namespace graphics {
  namespace Util {
    bool compileShader(GLuint shader) {
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
      return compileStatus == GL_TRUE;
    }

    bool linkProgram(GLuint program) {
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
      return linkStatus == GL_TRUE;
    }
  }
}
