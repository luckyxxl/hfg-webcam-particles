#include "main.hpp"

#include "Application.hpp"

Application::Application(Resources *resources, Webcam *_webcam) : webcam(_webcam) {
  webcam->getFrameSize(webcam_width, webcam_height);
  webcam_frame.resize(webcam_width * webcam_height * 3);

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

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindAttribLocation(program, 0, "v_position");
    glBindAttribLocation(program, 1, "v_rgb");
    glLinkProgram(program);

    glUseProgram(program);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

  glGenBuffers(1, &position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
  {
    std::vector<float> data(webcam_width * webcam_height * 2);
    for(uint32_t y=0; y<webcam_height; ++y) for(uint32_t x=0; x<webcam_width; ++x) {
      data[(y*webcam_width+x)*2+0] = x / (float)webcam_width;
      data[(y*webcam_width+x)*2+1] = y / (float)webcam_height;
    }
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
  }
  glGenBuffers(1, &rgb_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, rgb_buffer);
  glBufferData(GL_ARRAY_BUFFER, webcam_width * webcam_height * 3 * sizeof(float), nullptr, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glEnableVertexAttribArray(1);
}

Application::~Application() {
}

void Application::reshape(uint32_t width, uint32_t height) {
  glViewport(0, 0, width, height);
}

void Application::handleEvent(const SDL_Event &event) {
  switch(event.type) {
  }
}

void Application::update(float dt) {
	webcam->getFrame(webcam_frame.data());
	glBufferSubData(GL_ARRAY_BUFFER, 0, webcam_frame.size() * sizeof(float), webcam_frame.data());
}

void Application::render() {
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_POINTS, 0, webcam_width * webcam_height);

  {
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) printf("opengl error: %d\n", error);
  }
}
