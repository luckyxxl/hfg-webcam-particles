#pragma once

#include "graphics/Pipeline.hpp"
#include "Timeline.hpp"

class ParticleRenderer {
  public:
  ParticleRenderer(std::default_random_engine &random);

  void reset();
  void setTimeline(std::unique_ptr<Timeline> timeline);

  void update(float dt);
  void render();

  private:
  std::unique_ptr<Timeline> timeline;

  graphics::Pipeline graphicsPipeline;

  struct UniformElement {
    GLint location;
    UniformValueFunction value;
  };
  std::vector<UniformElement> uniforms;

  RendererState state;
  RenderProps props;
};
