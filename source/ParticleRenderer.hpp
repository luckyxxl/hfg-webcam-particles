#pragma once

#include "Timeline.hpp"
#include "SoundPlaylist.hpp"
#include "graphics/Pipeline.hpp"

class ParticleRenderer {
public:
  class GlobalState {
  public:
    void create(sound::Renderer *soundRenderer,
                const SampleLibrary *sampleLibrary,
                std::default_random_engine *random,
                const graphics::ScreenRectBuffer *screenRectBuffer);
    void destroy();
    void reshape(uint32_t width, uint32_t height);

  private:
    sound::Renderer *soundRenderer;
    const SampleLibrary *sampleLibrary;

    std::default_random_engine *random;

    const graphics::ScreenRectBuffer *screenRectBuffer;

    GLuint dummyVao;

    graphics::Framebuffer particleFramebuffer;
    graphics::Framebuffer accumulationFramebuffer;
    graphics::Framebuffer resultFramebuffer;

    graphics::Pipeline resultGraphicsPipeline;
    GLuint resultGraphicsPipeline_resultTexture_location;

    friend class ParticleRenderer;
  };

  void reset();
  void setTimeline(GlobalState &globalState, std::unique_ptr<Timeline> timeline);

  void enableSound(GlobalState &globalState);
  void disableSound(GlobalState &globalState);

  void update(GlobalState &globalState, float dt);
  void render(GlobalState &globalState, const RendererParameters &parameters);

  Timeline *getTimeline() { return timeline.get(); }

  Clock &getClock() { return state.clock; }

private:
  std::unique_ptr<Timeline> timeline;

  bool accumulationActive;

  graphics::Pipeline graphicsPipeline;
  graphics::Pipeline accGraphicsPipeline;

  GLint graphicsPipeline_particleTexture_location;
  GLint graphicsPipeline_backgroundTexture_location;

  GLint accGraphicsPipeline_particleTexture_location;
  GLint accGraphicsPipeline_historyTexture_location;

  struct UniformElement {
    GLint location;
    UniformValueFunction value;
  };
  std::vector<UniformElement> uniforms;
  std::vector<UniformElement> accUniforms;

  RendererState state;

  void loadUniforms(const std::vector<UniformElement> &uniforms, const RenderProps &props);

  SoundPlaylist soundPlaylist;
};
