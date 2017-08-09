#pragma once

#include "Timeline.hpp"
#include "graphics/Pipeline.hpp"

class ParticleRenderer {
public:
  class GlobalState {
  public:
    void create();
    void destroy();
    void reshape(uint32_t width, uint32_t height);

  private:
    graphics::ScreenRectBuffer screenRectBuffer;

    graphics::Framebuffer particleFramebuffer;
    graphics::Framebuffer accumulationFramebuffer;
    graphics::Framebuffer resultFramebuffer;

    graphics::Pipeline resultGraphicsPipeline;
    GLuint resultGraphicsPipeline_resultTexture_location;

    friend class ParticleRenderer;
  };

  void reset();
  void setTimeline(std::unique_ptr<Timeline> timeline);

  void update(float dt);
  void render(GlobalState &globalState, const RendererParameters &parameters);

  Timeline *getTimeline() { return timeline.get(); }

  Clock &getClock() { return state.clock; }

private:
  std::unique_ptr<Timeline> timeline;

  graphics::Pipeline graphicsPipeline;
  graphics::Pipeline accGraphicsPipeline;

  GLuint accGraphicsPipeline_particleTexture_location;
  GLuint accGraphicsPipeline_historyTexture_location;

  struct UniformElement {
    GLint location;
    UniformValueFunction value;
  };
  std::vector<UniformElement> uniforms;
  std::vector<UniformElement> accUniforms;

  RendererState state;

  void loadUniforms(const std::vector<UniformElement> &uniforms, const RenderProps &props);
};
