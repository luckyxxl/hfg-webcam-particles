#include "main.hpp"

#include "FaceBlitter.hpp"

void FaceBlitter::create(const graphics::ScreenRectBuffer *rectangle) {
  this->rectangle = rectangle;

  static const char *vertexShaderSource = R"glsl(
    #version 330 core
    uniform vec2 sourceCenter;
    uniform vec2 sourceExtend;
    uniform vec2 targetCenter;
    uniform vec2 targetExtend;

    layout(location=0) in vec2 position;

    out vec2 fragPosition;
    out vec2 sourceTexcoord;

    void main() {
      fragPosition = position;
      sourceTexcoord = sourceCenter + position * sourceExtend;
      vec2 p = targetCenter + position * targetExtend;
      gl_Position = vec4(mix(vec2(-1.f), vec2(1.f), p), 0.0, 1.0);
    }
  )glsl";

  static const char *fragmentShaderSource = R"glsl(
    #version 330 core
    uniform sampler2D source;

    in vec2 fragPosition;
    in vec2 sourceTexcoord;

    out vec4 frag_color;

    const float PI = 3.14159265;

    float vignette(float x) {
      const float blendStart = 0.9;
      const float blendEnd = 1.0;

      // linear ramp from blend start to blend end
      float l = clamp((x - blendStart) * (1.0 / (blendEnd - blendStart)), 0.0, 1.0);

      return 1.0 - l;
    }

    void main() {
      vec3 c = texture(source, sourceTexcoord).rgb;
      float a = vignette(min(length(fragPosition), 1.0));
      frag_color = vec4(c, a);
    }
  )glsl";

  pipeline.create(vertexShaderSource, fragmentShaderSource, graphics::Pipeline::BlendMode::Normal);

  sourceCenter_location = pipeline.getUniformLocation("sourceCenter");
  sourceExtend_location = pipeline.getUniformLocation("sourceExtend");
  targetCenter_location = pipeline.getUniformLocation("targetCenter");
  targetExtend_location = pipeline.getUniformLocation("targetExtend");
  source_location = pipeline.getUniformLocation("source");
}

void FaceBlitter::destroy() {
  pipeline.destroy();
}

void FaceBlitter::blit(graphics::Texture &source, glm::vec2 sourceMin, glm::vec2 sourceMax,
    graphics::Framebuffer &target, glm::vec2 targetMin, glm::vec2 targetMax) {
  target.bind();
  glViewport(0, 0, target.getWidth(), target.getHeight());

  pipeline.bind();

  glm::vec2 sourceCenter = (sourceMin + sourceMax) / 2.f;
  glm::vec2 sourceExtend = (sourceMax - sourceMin) / 2.f;
  glm::vec2 targetCenter = (targetMin + targetMax) / 2.f;
  glm::vec2 targetExtend = (targetMax - targetMin) / 2.f;

  glUniform2fv(sourceCenter_location, 1, &sourceCenter[0]);
  glUniform2fv(sourceExtend_location, 1, &sourceExtend[0]);
  glUniform2fv(targetCenter_location, 1, &targetCenter[0]);
  glUniform2fv(targetExtend_location, 1, &targetExtend[0]);

  source.bind(0);
  glUniform1i(source_location, 0);

  rectangle->draw();
}
