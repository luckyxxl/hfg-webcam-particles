#pragma once

#include "Timeline.hpp"
#include "graphics/Pipeline.hpp"

class ParticleRenderer {
public:
  void reset();
  void setTimeline(std::unique_ptr<Timeline> timeline);

  void update(float dt);
  void render(const RendererParameters &parameters);

private:
  std::unique_ptr<Timeline> timeline;

  graphics::Pipeline graphicsPipeline;

  struct UniformElement {
    GLint location;
    UniformValueFunction value;
  };
  std::vector<UniformElement> uniforms;

  RendererState state;
};
