#pragma once

#include "ParticleRenderer.hpp"
#include "effects/EffectRegistry.hpp"
#include "SampleLibrary.hpp"
#include "graphics/ParticleBuffer.hpp"
#include "ImageProvider.hpp"
#include "FaceBlitter.hpp"
#include "ParticleTextureToBuffer.hpp"

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

  FaceBlitter faceBlitter;
  ParticleTextureToBuffer particleTextureToBuffer;

  graphics::Pipeline overlayComposePilpeline;
  GLint overlayComposePilpeline_webcam_location;
  GLint overlayComposePilpeline_overlay_location;
  GLint overlayComposePilpeline_overlayVisibility_location;

  ParticleRenderer::GlobalState particleRendererGlobalState;

  ParticleRenderer standbyParticleRenderer;
  ParticleRenderer reactionParticleRenderer;

  enum class ReactionState {
    Inactive,
    FinishStandbyTimeline,
    RenderReactionTimeline,
  } reactionState = ReactionState::Inactive;

  graphics::Texture webcamTexture;
  graphics::Texture backgroundTexture;
  bool backgroundTextureIsSet = false;

  graphics::Framebuffer overlayFramebuffer;

  graphics::Framebuffer particleFramebuffer;

  graphics::ParticleBuffer particleBuffer;
};
