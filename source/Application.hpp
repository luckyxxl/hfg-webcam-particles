#pragma once

#include "ParticleRenderer.hpp"
#include "effects/EffectRegistry.hpp"
#include "SampleLibrary.hpp"
#include "ImageProvider.hpp"
#include "WebcamImageTransform.hpp"
#include "FaceBlitter.hpp"
#include "OverlayCompose.hpp"
#include "FinalComposite.hpp"
#include "ReactionTimelineRandomizer.hpp"

class Resources;
namespace sound {
class Renderer;
}
namespace graphics {
class Window;
}

class Application {
public:
  bool create(Resources *, graphics::Window *, sound::Renderer *);
  void destroy();

  bool handleEvents();
  void reshape(uint32_t width, uint32_t height);
  void update(float dt);
  void render();

private:
  graphics::Window *window;
  sound::Renderer *soundRenderer;

  EffectRegistry effectRegistry;
  SampleLibrary sampleLibrary;

  std::default_random_engine random;

  uint32_t screen_width, screen_height;

  ImageProvider imageProvider;

  graphics::ScreenRectBuffer screenRectBuffer;

  WebcamImageTransform webcamImageTransform;

  // FaceBlitter faceBlitter;

  // OverlayCompose overlayCompose;

  ParticleRenderer::GlobalState particleRendererGlobalState;

  ReactionTimelineRandomizer reactionTimelineRandomizer;

  ParticleRenderer standbyParticleRenderer;
  ParticleRenderer reactionParticleRenderer;

  ParticleRenderer::GlobalState backgroundParticleRendererGlobalState;
  ParticleRenderer backgroundParticleRenderer;

  enum class ReactionState {
    Inactive,
    FinishStandbyTimeline,
    RenderReactionTimeline,
  } reactionState = ReactionState::Inactive;

  float standbyBlitTimeout = 0.f;
  uint32_t standbyBlitCount = 0u;
  uint32_t standbyBlitTargetCount = 10u;

  graphics::Texture webcamInputTexture;
  graphics::Framebuffer webcamFramebuffer;
  graphics::Framebuffer backgroundFramebuffer;

  // graphics::Framebuffer particleSourceFramebuffer;

  graphics::Framebuffer particleOutputFramebuffer;
  graphics::Framebuffer backgroundParticleOutputFramebuffer;

  FinalComposite finalComposite;

#if WITH_EDIT_TOOLS
  bool paused = false;
  bool skipStandby = false;
#endif
};
