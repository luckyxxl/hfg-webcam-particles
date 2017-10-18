#include "main.hpp"

#include "FinalComposite.hpp"

void FinalComposite::create(const graphics::ScreenRectBuffer *rectangle) {
  this->rectangle = rectangle;

  static const char *vertexShaderSource = R"glsl(
    #version 330 core
    layout(location=0) in vec2 position;

    out vec2 screenCoord;
    out vec2 texcoord;

    void main() {
      screenCoord = position;
      texcoord = position * .5 + .5;
      gl_Position = vec4(position, 0., 1.);
    }
  )glsl";

  static const char *fragmentShaderSource = R"glsl(
    #version 330 core
    uniform sampler2D source;

    in vec2 screenCoord;
    in vec2 texcoord;

    out vec4 frag_color;

    float vignette(vec2 screenCoord) {
      float a = atan(screenCoord.y, screenCoord.x);

      // https://en.wikipedia.org/wiki/Squircle
      // https://thatsmaths.com/2016/07/14/squircles/  (Eq. 3)
      float s = sin(2 * a);
      float x = length(screenCoord) - s * s * .25;

      const float blendStart = 0.7;
      const float blendEnd = 0.95;

      // linear ramp from blend start to blend end
      float l = clamp((x - blendStart) * (1.0 / (blendEnd - blendStart)), 0.0, 1.0);

      return 1.0 - l*l;
    }

    void main() {
      //vec3 c = texture(source, texcoord).rgb;
      vec3 c = texelFetch(source, ivec2(gl_FragCoord.xy), 0).rgb;
      frag_color = vec4(c * vignette(screenCoord), 0.);
    }
  )glsl";

  pipeline.create(vertexShaderSource, fragmentShaderSource, graphics::Pipeline::BlendMode::None);

  pipeline_source_location = pipeline.getUniformLocation("source");
}

void FinalComposite::destroy() {
  pipeline.destroy();
}

void FinalComposite::draw(graphics::Texture &source, uint32_t screen_width, uint32_t screen_height) {
  graphics::Framebuffer::unbind();
  glViewport(0, 0, screen_width, screen_height);
  pipeline.bind();
  source.bind(0u);
  glUniform1i(pipeline_source_location, 0u);
  rectangle->draw();
}
