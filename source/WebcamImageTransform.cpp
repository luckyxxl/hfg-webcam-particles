#include "main.hpp"

#include "WebcamImageTransform.hpp"

void WebcamImageTransform::create(const graphics::ScreenRectBuffer *rectangle,
      uint32_t input_width, uint32_t input_height,
      uint32_t output_width, uint32_t output_height) {
  this->rectangle = rectangle;
  this->input_width = input_width;
  this->input_height = input_height;
  this->output_width = output_width;
  this->output_height = output_height;

  const auto overscan = ((float)input_width / input_height) / ((float)output_width / output_height);

  // column-major!!!
  transform = glm::mat3(-.95f, 0.f, 0.f,
                        0.f, -.95f * overscan, 0.f,
                        1.f, 1.f, 1.f);
  inverseTransform = glm::inverse(transform);

  static const char *vertexShaderSource = R"glsl(
    #version 330 core
    layout(location=0) in vec2 position;

    uniform mat3 transform;

    out vec2 texcoord;

    void main() {
      vec2 pixelTexcoord = position * .5 + .5;
      texcoord = vec2(transform * vec3(pixelTexcoord, 1.));
      gl_Position = vec4(position, 0., 1.);
    }
  )glsl";

  static const char *fragmentShaderSource = R"glsl(
    #version 330 core
    uniform sampler2D source;

    in vec2 texcoord;

    out vec4 frag_color;

    void main() {
      vec3 c = texture(source, texcoord).bgr;
      frag_color = vec4(c, 0.);
    }
  )glsl";

  pipeline.create(vertexShaderSource, fragmentShaderSource, graphics::Pipeline::BlendMode::None);

  pipeline_transform_location = pipeline.getUniformLocation("transform");
  pipeline_source_location = pipeline.getUniformLocation("source");
}

void WebcamImageTransform::destroy() {
  pipeline.destroy();
}

void WebcamImageTransform::draw(graphics::Texture &input, graphics::Framebuffer &output) {
  assert(input.getWidth() == input_width);
  assert(input.getHeight() == input_height);
  assert(output.getWidth() == output_width);
  assert(output.getHeight() == output_height);

  output.bind();
  glViewport(0, 0, output.getWidth(), output.getHeight());
  pipeline.bind();
  glUniformMatrix3fv(pipeline_transform_location, 1, GL_FALSE, &transform[0][0]);
  input.bind(0u);
  glUniform1i(pipeline_source_location, 0u);
  rectangle->draw();
}
