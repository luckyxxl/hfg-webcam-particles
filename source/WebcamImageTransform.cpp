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

  //const auto overscan = ((float)input_width / input_height) / ((float)output_width / output_height);

  // column-major!!!
  transform = glm::mat3(-1.f, 0.f, 0.f,
                        0.f, -1.f, 0.f,
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

    uniform vec3 imageParameters;

    in vec2 texcoord;

    out vec4 frag_color;

    const float PI = 3.14159265;

    vec3 rgb2hsv(vec3 rgb) {
      float cmin = min(rgb.r, min(rgb.g, rgb.b));
      float cmax = max(rgb.r, max(rgb.g, rgb.b));
      float d = cmax - cmin;
      float eps = 0.00001;
      if (d < eps || cmax < eps) {
        return vec3(0, 0, cmax);
      }

      float _h;
      if (cmax == rgb.r) {
        _h = (rgb.g - rgb.b) / d;
        if (_h < 0.) {
          _h += 6.;
        }
      } else if (cmax == rgb.g) {
        _h = ((rgb.b - rgb.r) / d) + 2.;
      } else {
        _h = ((rgb.r - rgb.g) / d) + 4.;
      }

      return vec3(_h * 60. * (PI / 180.), d / cmax, cmax);
    }

    vec3 hsv2rgb(vec3 hsv) {
      // rapidtables.com/convert/color/hsv-to-rgb.htm
      float _h = hsv.x * 180. / PI;
      float _c = hsv.y * hsv.z;
      float _x = _c * (1. - abs(mod(_h / 60., 2.) - 1.));
      float _m = hsv.z - _c;
      vec3 _r;
      if (/* 0. <= _h && */ _h < 60.) {
        _r.r = _c;
        _r.g = _x;
        _r.b = 0.;
      } else if (_h < 120.) {
        _r.r = _x;
        _r.g = _c;
        _r.b = 0.;
      } else if (_h < 180.) {
        _r.r = 0.;
        _r.g = _c;
        _r.b = _x;
      } else if (_h < 240.) {
        _r.r = 0.;
        _r.g = _x;
        _r.b = _c;
      } else if (_h < 300.) {
        _r.r = _x;
        _r.g = 0.;
        _r.b = _c;
      } else /* if (_h < 360.) */ {
        _r.r = _c;
        _r.g = 0.;
        _r.b = _x;
      }
      return _r + vec3(_m);
    }

    void main() {
      vec3 rgb = texture(source, texcoord).bgr;
      rgb = imageParameters[0] * rgb + imageParameters[1]; // increase overall brightness
      vec3 hsv = rgb2hsv(rgb);
      hsv.y = hsv.y * imageParameters[2]; // increase saturation
      rgb = clamp(hsv2rgb(hsv), 0., 1.);
      frag_color = vec4(rgb, 0.);
    }
  )glsl";

  pipeline.create(vertexShaderSource, fragmentShaderSource, graphics::Pipeline::BlendMode::None);

  pipeline_transform_location = pipeline.getUniformLocation("transform");
  pipeline_source_location = pipeline.getUniformLocation("source");
  pipeline_imageParameters_location = pipeline.getUniformLocation("imageParameters");
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
  glUniform3f(pipeline_imageParameters_location, brightnessMul, brightnessAdd, saturation);
  rectangle->draw();
}
