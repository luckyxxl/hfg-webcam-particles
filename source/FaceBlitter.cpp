#include "main.hpp"

#include "FaceBlitter.hpp"

constexpr float FADE_TIME = 1000.f;

void FaceBlitter::create(const graphics::ScreenRectBuffer *rectangle,
    uint32_t webcam_width, uint32_t webcam_height) {
  this->rectangle = rectangle;

  blitOperations.reserve(8u);

  static const char *blitToBufferPipeline_vertexShaderSource = R"glsl(
    #version 330 core
    uniform vec2 sourceCenter;
    uniform vec2 sourceExtend;
    uniform vec2 targetCenter;
    uniform vec2 targetExtend;

    layout(location=0) in vec2 position;

    out vec2 fragPosition;
    out vec2 sourceTexcoord;

    void main() {
      // transparently (both in terms of visibility and parameters) enlarge rectangle by .01
      fragPosition = position * 1.005;
      sourceTexcoord = sourceCenter + position * (sourceExtend + 0.005);
      vec2 p = targetCenter + position * (targetExtend + 0.005);
      gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
    }
  )glsl";

  static const char *blitToBufferPipeline_fragmentShaderSource = R"glsl(
    #version 330 core
    uniform sampler2D source;
    uniform sampler2D background;

    in vec2 fragPosition;
    in vec2 sourceTexcoord;

    out vec4 frag_color;

    const float PI = 3.14159265;

    float vignette(float x) {
      const float blendStart = 0.5;
      const float blendEnd = 1.0;

      // linear ramp from blend start to blend end
      float l = clamp((x - blendStart) * (1.0 / (blendEnd - blendStart)), 0.0, 1.0);

      return 1.0 - l*l;
    }

    float key(vec3 front, vec3 back) {
#if 1
      return 1.;
#else
      float d = length(front - back);
      return clamp(d * 4., 0., 1.);
#endif
    }

    void main() {
      vec3 c = texture(source, sourceTexcoord).rgb;
      vec3 b = texture(background, sourceTexcoord).rgb;
      float a = vignette(length(fragPosition)) * key(c, b);
      frag_color = vec4(c, a);
    }
  )glsl";

  static const char *blitToResultPipeline_vertexShaderSource = R"glsl(
    #version 330 core
    uniform vec2 sourceCenter;
    uniform vec2 sourceExtend;
    uniform vec2 targetCenter;
    uniform vec2 targetExtend;

    layout(location=0) in vec2 position;

    out vec2 sourceTexcoord;

    void main() {
      sourceTexcoord = sourceCenter + position * sourceExtend;
      vec2 p = targetCenter + position * targetExtend;
      gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
    }
  )glsl";

  static const char *blitToResultPipeline_fragmentShaderSource = R"glsl(
    #version 330 core
    uniform sampler2D source;
    uniform float visibility;

    in vec2 sourceTexcoord;

    out vec4 frag_color;

    void main() {
      vec4 color = texture(source, sourceTexcoord);
      frag_color = vec4(color.rgb, color.a * visibility);
    }
  )glsl";

  blitToBufferPipeline.create(blitToBufferPipeline_vertexShaderSource,
    blitToBufferPipeline_fragmentShaderSource, graphics::Pipeline::BlendMode::None);

  blitToBufferPipeline_sourceCenter_location = blitToBufferPipeline.getUniformLocation("sourceCenter");
  blitToBufferPipeline_sourceExtend_location = blitToBufferPipeline.getUniformLocation("sourceExtend");
  blitToBufferPipeline_targetCenter_location = blitToBufferPipeline.getUniformLocation("targetCenter");
  blitToBufferPipeline_targetExtend_location = blitToBufferPipeline.getUniformLocation("targetExtend");
  blitToBufferPipeline_source_location = blitToBufferPipeline.getUniformLocation("source");
  blitToBufferPipeline_background_location = blitToBufferPipeline.getUniformLocation("background");

  blitToResultPipeline.create(blitToResultPipeline_vertexShaderSource,
    blitToResultPipeline_fragmentShaderSource, graphics::Pipeline::BlendMode::Normal);

  blitToResultPipeline_sourceCenter_location = blitToResultPipeline.getUniformLocation("sourceCenter");
  blitToResultPipeline_sourceExtend_location = blitToResultPipeline.getUniformLocation("sourceExtend");
  blitToResultPipeline_targetCenter_location = blitToResultPipeline.getUniformLocation("targetCenter");
  blitToResultPipeline_targetExtend_location = blitToResultPipeline.getUniformLocation("targetExtend");
  blitToResultPipeline_source_location = blitToResultPipeline.getUniformLocation("source");
  blitToResultPipeline_visibility_location = blitToResultPipeline.getUniformLocation("visibility");

  facesBuffer.create(webcam_width, webcam_height);
  overlayFramebuffer.create(webcam_width, webcam_height);
  resultFramebuffer.create(webcam_width, webcam_height);

  clear();
}

void FaceBlitter::destroy() {
  resultFramebuffer.destroy();
  overlayFramebuffer.destroy();
  facesBuffer.destroy();
  blitToResultPipeline.destroy();
  blitToBufferPipeline.destroy();
}

void FaceBlitter::blit(graphics::Texture &source, glm::vec2 sourceMin, glm::vec2 sourceMax,
    glm::vec2 targetMin, glm::vec2 targetMax, graphics::Texture &background) {
  const glm::vec2 sourceCenter = (sourceMin + sourceMax) / 2.f;
  const glm::vec2 sourceSize = sourceMax - sourceMin;
  const glm::vec2 sourceExtend = sourceSize / 2.f;
  const glm::vec2 targetCenter = (targetMin + targetMax) / 2.f;
  const glm::vec2 targetSize = targetMax - targetMin;
  const glm::vec2 targetExtend = targetSize / 2.f;

  // the rectangle is enlarged by .01 in the vertex shader
  const glm::vec2 bufferSize = targetSize + .01f;

  if(nextBufferOrigin.x + bufferSize.x > 1.f) {
    nextBufferOrigin.x = 0.f;
    nextBufferOrigin.y += currentBufferRowYSize;
    currentBufferRowYSize = 0.f;
  }
  if(nextBufferOrigin.y + bufferSize.y > 1.f) {
    nextBufferOrigin.x = 0.f;
    nextBufferOrigin.y = 0.f;
    currentBufferRowYSize = 0.f;
  }

  assert(nextBufferOrigin.x + bufferSize.x <= 1.f);
  assert(nextBufferOrigin.y + bufferSize.y <= 1.f);

  // the rectangle is enlarged by .01 in the vertex shader
  const glm::vec2 bufferCenter = nextBufferOrigin + targetExtend + .005f;

  nextBufferOrigin.x += bufferSize.x;
  currentBufferRowYSize = glm::max(currentBufferRowYSize, bufferSize.y);

  {
    BlitOperation blitOperation;
    blitOperation.bufferCenter = bufferCenter;
    blitOperation.targetCenter = targetCenter;
    blitOperation.extend = targetExtend;
    blitOperation.time = 0.f;
    blitOperations.push_back(blitOperation);
  }

  facesBuffer.bind();
  glViewport(0, 0, facesBuffer.getWidth(), facesBuffer.getHeight());

  blitToBufferPipeline.bind();

  glUniform2fv(blitToBufferPipeline_sourceCenter_location, 1, &sourceCenter[0]);
  glUniform2fv(blitToBufferPipeline_sourceExtend_location, 1, &sourceExtend[0]);
  glUniform2fv(blitToBufferPipeline_targetCenter_location, 1, &bufferCenter[0]);
  glUniform2fv(blitToBufferPipeline_targetExtend_location, 1, &targetExtend[0]);

  source.bind(0);
  glUniform1i(blitToBufferPipeline_source_location, 0);
  background.bind(1);
  glUniform1i(blitToBufferPipeline_background_location, 1);

  rectangle->draw();
}

void FaceBlitter::update(float dt) {
  for(auto &op : blitOperations) {
    op.time += dt;
  }
}

void FaceBlitter::draw() {
  assert(std::is_sorted(blitOperations.begin(), blitOperations.end(),
    [](const BlitOperation &a, const BlitOperation &b) { return a.time >= b.time; }));

  if(blitOperations.empty()) return;

  glViewport(0, 0, overlayFramebuffer.getWidth(), overlayFramebuffer.getHeight());
  assert(resultFramebuffer.getWidth() == overlayFramebuffer.getWidth());
  assert(resultFramebuffer.getHeight() == overlayFramebuffer.getHeight());

  blitToResultPipeline.bind();

  glUniform1i(blitToResultPipeline_source_location, 0);
  glUniform1f(blitToResultPipeline_visibility_location, 1.f);

  if(blitOperations.front().time >= FADE_TIME) {
    overlayFramebuffer.bind();

    facesBuffer.getTexture().bind(0);

    do {
      const auto &op = blitOperations.front();

      glUniform2fv(blitToResultPipeline_sourceCenter_location, 1, &op.bufferCenter[0]);
      glUniform2fv(blitToResultPipeline_sourceExtend_location, 1, &op.extend[0]);
      glUniform2fv(blitToResultPipeline_targetCenter_location, 1, &op.targetCenter[0]);
      glUniform2fv(blitToResultPipeline_targetExtend_location, 1, &op.extend[0]);
      rectangle->draw();

      blitOperations.erase(blitOperations.begin());
    } while(!blitOperations.empty() && blitOperations.front().time >= FADE_TIME);
  }

  resultFramebuffer.bind();

  glDisable(GL_BLEND); // temporary disable blending because we want to simply copy the overlayFB

  glUniform2f(blitToResultPipeline_sourceCenter_location, .5f, .5f);
  glUniform2f(blitToResultPipeline_sourceExtend_location, .5f, .5f);
  glUniform2f(blitToResultPipeline_targetCenter_location, .5f, .5f);
  glUniform2f(blitToResultPipeline_targetExtend_location, .5f, .5f);
  overlayFramebuffer.getTexture().bind(0);
  rectangle->draw();

  glEnable(GL_BLEND);

  if(!blitOperations.empty()) {
    facesBuffer.getTexture().bind(0);

    for(const auto &op : blitOperations) {
      glUniform2fv(blitToResultPipeline_sourceCenter_location, 1, &op.bufferCenter[0]);
      glUniform2fv(blitToResultPipeline_sourceExtend_location, 1, &op.extend[0]);
      glUniform2fv(blitToResultPipeline_targetCenter_location, 1, &op.targetCenter[0]);
      glUniform2fv(blitToResultPipeline_targetExtend_location, 1, &op.extend[0]);
      glUniform1f(blitToResultPipeline_visibility_location, op.time / FADE_TIME);
      rectangle->draw();
    }
  }
}

void FaceBlitter::clear() {
  blitOperations.clear();

  nextBufferOrigin = glm::vec2(0.f, 0.f);
  currentBufferRowYSize = 0.f;

  overlayFramebuffer.clear();
  resultFramebuffer.clear();
}
