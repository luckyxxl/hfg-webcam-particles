#include "main.hpp"

#include "Application.hpp"

struct Particle {
  float position[2];
  float rgb[3];
  float hsv[3];
};

Application::Application(Resources *resources, Webcam *_webcam) : webcam(_webcam) {
  webcam->getFrameSize(webcam_width, webcam_height);
  webcam_frame.resize(webcam_width * webcam_height * 3);

  glEnable(GL_PROGRAM_POINT_SIZE);

  {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    auto vertex_shader_source = resources->readWholeFile("test.glslv");
    auto vs = vertex_shader_source.c_str();
    glShaderSource(vertex_shader, 1, &vs, nullptr);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    auto fragment_shader_source = resources->readWholeFile("test.glslf");
    auto fs = fragment_shader_source.c_str();
    glShaderSource(fragment_shader, 1, &fs, nullptr);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindAttribLocation(program, 0, "v_position");
    glBindAttribLocation(program, 1, "v_rgb");
    glBindAttribLocation(program, 2, "v_hsv");
    glLinkProgram(program);

    glUseProgram(program);

    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
  }

  time_location = glGetUniformLocation(program, "time");

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, webcam_width * webcam_height * sizeof(Particle), nullptr, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, rgb)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, hsv)));
  glEnableVertexAttribArray(2);
}

Application::~Application() {
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array);
  glDeleteProgram(program);
}

void Application::reshape(uint32_t width, uint32_t height) {
  glViewport(0, 0, width, height);
}

void Application::handleEvent(const SDL_Event &event) {
  switch(event.type) {
  }
}

static void rgb2Hsv(float *hsv, const float *rgb) {
  const auto cMax = std::max(std::max(rgb[0], rgb[1]), rgb[2]);
  const auto cMin = std::min(std::min(rgb[0], rgb[1]), rgb[2]);
  const auto d = cMax - cMin;

  if(d < 0.00001f || cMax < 0.00001f) {
    hsv[0] = 0.f; hsv[1] = 0.f; hsv[2] = cMax;
    return;
  }

  float h;
  if(cMax == rgb[0]) {
    h = (rgb[1] - rgb[2]) / d;
    if(h < 0) h += 6.f;
  } else if(cMax == rgb[1]) {
    h = (rgb[2] - rgb[0]) / d + 2.f;
  } else {
    h = (rgb[0] - rgb[1]) / d + 4.f;
  }

  hsv[0] = h * 60.f * PI / 180.f;
  hsv[1] = d / cMax;
  hsv[2] = cMax;
}

void Application::update(float dt) {
  webcam->getFrame(webcam_frame.data());
  {
    std::vector<Particle> data(webcam_width * webcam_height);
    for(size_t i=0; i<data.size(); ++i) {
      auto &particle = data[i];
      auto x = i % webcam_width;
      auto y = i / webcam_width;
      particle.position[0] = x / (float)webcam_width;
      particle.position[1] = y / (float)webcam_height;
      particle.rgb[0] = webcam_frame[(y*webcam_width+x)*3+0];
      particle.rgb[1] = webcam_frame[(y*webcam_width+x)*3+1];
      particle.rgb[2] = webcam_frame[(y*webcam_width+x)*3+2];
      rgb2Hsv(particle.hsv, particle.rgb);
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(Particle), data.data());
  }
}

void Application::render() {
  glClear(GL_COLOR_BUFFER_BIT);

  glUniform1f(time_location, SDL_GetTicks() / 1000.f);
  glDrawArrays(GL_POINTS, 0, webcam_width * webcam_height);

  {
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) printf("opengl error: %d\n", error);
  }
}
