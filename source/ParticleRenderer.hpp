#pragma once

class ParticleRenderer {
  public:
  void setTimeline(const Timeline &timeline);

  void update(float dt);
  void render();

  private:
  graphics::Pipeline graphicsPipeline;

  struct UniformElement {
    std::string name;
    GLint location;
  };
  std::vector<UniformElement> uniforms;
};
