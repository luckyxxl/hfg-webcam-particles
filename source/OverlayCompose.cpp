#include "main.hpp"

#include "OverlayCompose.hpp"

void OverlayCompose::create(const graphics::ScreenRectBuffer *rectangle) {
  this->rectangle = rectangle;

  pipeline.create(R"glsl(
    #version 330 core
    layout(location=0) in vec2 position;
    void main() { gl_Position = vec4(position, 0.0, 1.0); }
    )glsl", R"glsl(
    #version 330 core
    uniform sampler2D webcam;
    uniform sampler2D overlay;
    uniform float overlayVisibility;
    out vec4 frag_color;
    void main() {
      vec4 w = texelFetch(webcam, ivec2(gl_FragCoord.xy), 0);
      vec4 o = texelFetch(overlay, ivec2(gl_FragCoord.xy), 0);
      float a = o.a * overlayVisibility;
      frag_color = vec4(mix(w.rgb, o.rgb, a), a);
    }
    )glsl", graphics::Pipeline::BlendMode::None);
  pipeline_webcam_location = pipeline.getUniformLocation("webcam");
  pipeline_overlay_location = pipeline.getUniformLocation("overlay");
  pipeline_overlayVisibility_location = pipeline.getUniformLocation("overlayVisibility");
}

void OverlayCompose::destroy() {
  pipeline.destroy();
}

void OverlayCompose::draw(graphics::Texture &input, graphics::Texture &overlay, float overlayVisibility, graphics::Framebuffer &output) {
  output.bind();
  glViewport(0, 0, output.getWidth(), output.getHeight());
  pipeline.bind();
  input.bind(0u);
  glUniform1i(pipeline_webcam_location, 0u);
  overlay.bind(1u);
  glUniform1i(pipeline_overlay_location, 1u);
  glUniform1f(pipeline_overlayVisibility_location, overlayVisibility);
  rectangle->draw();
}
