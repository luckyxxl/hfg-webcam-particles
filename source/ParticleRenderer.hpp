#pragma once

#include "Timeline.hpp"
#include "graphics/Pipeline.hpp"

class ParticleRenderer {
public:
  void reset();
  void setTimeline(std::unique_ptr<Timeline> timeline);

  void update(float dt);
  void render(const RendererParameters &parameters);

  Timeline *getTimeline() { return timeline.get(); }

  Clock &getClock() { return state.clock; }

private:
  std::unique_ptr<Timeline> timeline;

  graphics::Pipeline graphicsPipeline;
  graphics::Pipeline accGraphicsPipeline;
  graphics::Pipeline resultGraphicsPipeline;

  struct UniformElement {
    GLint location;
    UniformValueFunction value;
  };
  std::vector<UniformElement> uniforms;
  std::vector<UniformElement> accUniforms;

  RendererState state;

  void loadUniforms(const std::vector<UniformElement> &uniforms, const RenderProps &props);
  void loadTextureUniform(const graphics::Pipeline &pipeline, const char *name, graphics::Texture &texture, uint32_t unit);
};
