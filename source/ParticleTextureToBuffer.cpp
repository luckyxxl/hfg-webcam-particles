#include "main.hpp"

#include "ParticleTextureToBuffer.hpp"
#include "graphics/Util.hpp"

void ParticleTextureToBuffer::create() {
  glGenVertexArrays(1, &vao);

  static const char *vertexShaderSource = R"glsl(
  #version 330 core
  uniform int width;
  uniform int height;
  uniform sampler2D particleTexture;
  uniform sampler2D backgroundTexture;

  out vec2 position;
  out vec3 rgb;
  out vec3 hsv;
  out float foregroundMask;

  const float PI = 3.14159265;

  vec3 rgb2hsv(vec3 rgb) {
      float cMax = max(max(rgb.r, rgb.g), rgb.b);
      float cMin = min(min(rgb.r, rgb.g), rgb.b);
      float d = cMax - cMin;

      if (d < 0.00001 || cMax < 0.00001) {
        return vec3(0., 0., cMax);
      }

      float h;
      if (cMax == rgb.r) {
        h = (rgb.g - rgb.b) / d;
        if (h < 0) h += 6.;
      } else if (cMax == rgb.g) {
        h = (rgb.b - rgb.r) / d + 2.;
      } else {
        h = (rgb.r - rgb.g) / d + 4.;
      }

      return vec3(h * 60. * PI / 180., d / cMax, cMax);
  }

  void main() {
    ivec2 pixelPosition = ivec2(gl_VertexID % width, gl_VertexID / width);

    position = pixelPosition / vec2(width, height);
    position.y = 1 - position.y;

    rgb = texelFetch(particleTexture, pixelPosition, 0).bgr;

    hsv = rgb2hsv(rgb);

    vec3 backgroundDelta = rgb - texelFetch(backgroundTexture, pixelPosition, 0).bgr;
    float backgroundDifference = dot(backgroundDelta, backgroundDelta);
    foregroundMask = clamp((backgroundDifference - .2) * 100.f, 0., 1.);
  }
  )glsl";

  GLuint shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shader, 1, &vertexShaderSource, nullptr);
  if(!graphics::Util::compileShader(shader)) return;

  program = glCreateProgram();
  glAttachShader(program, shader);

  static const char *varyings[] = {
    "position",
    "rgb",
    "hsv",
    "foregroundMask",
  };
  glTransformFeedbackVaryings(program, sizeof(varyings)/sizeof(*varyings), varyings, GL_INTERLEAVED_ATTRIBS);

  graphics::Util::linkProgram(program);

  glDeleteShader(shader);

  width_location = glGetUniformLocation(program, "width");
  height_location = glGetUniformLocation(program, "height");
  particleTexture_location = glGetUniformLocation(program, "particleTexture");
  backgroundTexture_location = glGetUniformLocation(program, "backgroundTexture");
}

void ParticleTextureToBuffer::destroy() {
  glDeleteProgram(program);
  glDeleteVertexArrays(1, &vao);
}

void ParticleTextureToBuffer::render(uint32_t width, uint32_t height,
  graphics::Texture &particleTexture, graphics::Texture &backgroundTexture,
  graphics::ParticleBuffer &particleBuffer) {
  glBindVertexArray(vao);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer.getVertexBuffer());

  glUseProgram(program);

  glUniform1i(width_location, width);
  glUniform1i(height_location, height);
  particleTexture.bind(0);
  glUniform1i(particleTexture_location, 0);
  backgroundTexture.bind(1);
  glUniform1i(backgroundTexture_location, 1);

  glBeginTransformFeedback(GL_POINTS);
  glDrawArrays(GL_POINTS, 0, width*height);
  glEndTransformFeedback();

#if 0
  {
    std::vector<float> data(512);
    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer.getVertexBuffer());
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(float), data.data());

#if 1
    for(auto f : data) {
      std::cout << f << " ";
    }
    std::cout << "\n";
#endif
#if 0
    for(auto i=8; i<data.size(); i+=9) {
      std::cout << data[i] << " ";
    }
    std::cout << "\n";
#endif
  }
#endif
}
