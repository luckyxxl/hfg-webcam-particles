#include "main.hpp"

#include "Application.hpp"

#include "Resources.hpp"
#include "effects/ConvergeCircleEffect.hpp"
#include "effects/ConvergePointEffect.hpp"
#include "effects/HueDisplaceEffect.hpp"
#include "effects/ParticleSizeByHueEffect.hpp"
#include "effects/SmearEffect.hpp"
#include "effects/SmoothTrailsEffect.hpp"
#include "effects/StandingWaveEffect.hpp"
#include "effects/TrailsEffect.hpp"
#include "effects/WaveEffect.hpp"
#include "graphics/Window.hpp"
#include "sound/Renderer.hpp"

#include <stb_image_write.h>

bool Application::create(Resources *resources, graphics::Window *window,
                         sound::Renderer *soundRenderer) {
  this->window = window;
  this->soundRenderer = soundRenderer;

  effectRegistry.registerEffect<ConvergeCircleEffect>();
  effectRegistry.registerEffect<ConvergePointEffect>();
  effectRegistry.registerEffect<HueDisplaceEffect>();
  effectRegistry.registerEffect<ParticleSizeByHueEffect>();
  effectRegistry.registerEffect<SmearEffect>();
  effectRegistry.registerEffect<SmoothTrailsEffect>();
  effectRegistry.registerEffect<StandingWaveEffect>();
  effectRegistry.registerEffect<TrailsEffect>();
  effectRegistry.registerEffect<WaveEffect>();

  if(!sampleLibrary.create(random, resources)) {
    return false;
  }

  soundRenderer->play(sampleLibrary.getBackgroundLoop(),
                      sound::Renderer::PlayParameters().setLooping(true));

  if(!imageProvider.create(*resources) || !imageProvider.start()) {
    return false;
  }

  screenRectBuffer.create();

  faceBlitter.create(&screenRectBuffer);

  overlayComposePilpeline.create(R"glsl(
    #version 330 core
    layout(location=0) in vec2 position;
    void main() { gl_Position = vec4(position, 0.0, 1.0); }
    )glsl", R"glsl(
    #version 330 core
    uniform sampler2D webcam;
    uniform sampler2D overlay;
    uniform float overlayVisibility;
    out vec4 frag_color;
    void main() {
      vec4 w = texelFetch(webcam, ivec2(gl_FragCoord.xy), 0);
      vec4 o = texelFetch(overlay, ivec2(gl_FragCoord.xy), 0);
      frag_color = vec4(mix(w.rgb, o.rgb, o.a * overlayVisibility), 0.0);
    }
    )glsl", graphics::Pipeline::BlendMode::None);
  overlayComposePilpeline_webcam_location = overlayComposePilpeline.getUniformLocation("webcam");
  overlayComposePilpeline_overlay_location = overlayComposePilpeline.getUniformLocation("overlay");
  overlayComposePilpeline_overlayVisibility_location = overlayComposePilpeline.getUniformLocation("overlayVisibility");

  particleRendererGlobalState.create(soundRenderer, &sampleLibrary, &random, &screenRectBuffer);

  {
    auto timeline = std::make_unique<Timeline>(&effectRegistry);

    auto hueDisplace = timeline->emplaceEffectInstance<HueDisplaceEffect>();
    hueDisplace->timeBegin = 0.f;
    hueDisplace->timeEnd = 2000.f;
    hueDisplace->distance = .01f;
    hueDisplace->scaleByForegroundMask = 1.f;

    standbyParticleRenderer.setTimeline(particleRendererGlobalState, std::move(timeline));
  }

  {
    auto timeline = std::make_unique<Timeline>(&effectRegistry);
    timeline->setFixedPeriod(6000.f);

    timeline->emplaceEffectInstance<ConvergeCircleEffect>();
    timeline->emplaceEffectInstance<ConvergePointEffect>();
    timeline->emplaceEffectInstance<HueDisplaceEffect>();
    timeline->emplaceEffectInstance<StandingWaveEffect>();
    timeline->emplaceEffectInstance<WaveEffect>();

    auto accum = timeline->emplaceEffectInstance<TrailsEffect>();
    accum->timeBegin = 0.f;
    accum->timeEnd = timeline->getPeriod();
    accum->fadeIn = 1000.f;
    accum->fadeOut = 1000.f;
    accum->strength = .7f;

    reactionParticleRenderer.setTimeline(particleRendererGlobalState, std::move(timeline));

    reactionParticleRenderer.getClock().disableLooping();
    reactionParticleRenderer.getClock().pause();
  }

  webcamTexture.create(imageProvider.width(), imageProvider.height());
  backgroundTexture.create(imageProvider.width(), imageProvider.height());
  overlayFramebuffer.create(imageProvider.width(), imageProvider.height());
  particleFramebuffer.create(imageProvider.width(), imageProvider.height());

  // prevent undefined images at startup
  {
    std::vector<uint8_t> pixels(imageProvider.width() * imageProvider.height() * 3, 0.f);
    webcamTexture.setImage(imageProvider.width(), imageProvider.height(), pixels.data());
  }

  overlayFramebuffer.clear();

  return true;
}

void Application::destroy() {
  particleFramebuffer.destroy();
  overlayFramebuffer.destroy();
  backgroundTexture.destroy();
  webcamTexture.destroy();

  reactionParticleRenderer.reset();
  standbyParticleRenderer.reset();

  particleRendererGlobalState.destroy();

  overlayComposePilpeline.destroy();

  faceBlitter.destroy();

  screenRectBuffer.destroy();

  imageProvider.stop();

  soundRenderer->killAllVoices();

  sampleLibrary.destroy();
}

void Application::reshape(uint32_t width, uint32_t height) {
  screen_width = width;
  screen_height = height;

  glViewport(0, 0, width, height);

  particleRendererGlobalState.reshape(width, height);
}

bool Application::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      return false;
      break;
    }
    case SDL_WINDOWEVENT: {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_RESIZED: {
        reshape(event.window.data1, event.window.data2);
        break;
      }
      }
      break;
    }
    case SDL_KEYDOWN: {
      switch (event.key.keysym.scancode) {
      /*
      case SDL_SCANCODE_1:
      soundRenderer->play(&whooshSamples[0]);
      break;
      case SDL_SCANCODE_2:
      soundRenderer->play(&whooshSamples[1]);
      break;
      case SDL_SCANCODE_3:
      soundRenderer->play(&whooshSamples[2]);
      break;
      case SDL_SCANCODE_4:
      soundRenderer->play(&whooshSamples[3]);
      break;
      case SDL_SCANCODE_5:
      soundRenderer->play(&whooshSamples[4]);
      break;
      */
      case SDL_SCANCODE_F1:
      // screenshot
      {
        std::vector<uint8_t> pixels(screen_width * screen_height * 3);
        glReadPixels(0, 0, screen_width, screen_height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        stbi_write_png("screenshot.png", screen_width, screen_height, 3, pixels.data(), 0);
      }
      break;
      default:
        break;
      }
    }
    case SDL_MOUSEBUTTONDOWN: {
      if (event.button.button == SDL_BUTTON_LEFT && event.button.clicks == 2) {
        window->toggleFullscreen(); // FIXME handle return value
      }
      break;
    }
    }
  }
  return true;
}

static void randomizeTimeline(Timeline *timeline,
                              std::default_random_engine &random) {
  assert(timeline->hasFixedPeriod());
  const auto period = timeline->getPeriod();
  const auto minLength = 2000.f;

  timeline->forEachInstance([&](IEffect &i) {
    if(i.isAccumulationEffect()) return;

    i.timeBegin = std::uniform_real_distribution<float>(0.f, period - minLength)
                    (random);
    i.timeEnd = i.timeBegin + std::uniform_real_distribution<float>(minLength,
                                period - i.timeBegin)(random);
    i.randomizeConfig(random);
  });
}

void Application::update(float dt) {
  lastBackgroundUpdateTime += dt;

  if(reactionState == ReactionState::Inactive) {
    standbyBlitTimeout -= dt;
  }

  ImageData imgData; // FIXME this could be a member variable to cache previous allocations
  imageProvider >> imgData;
  if (!imgData.empty()) {
    webcamTexture.setImage(imgData.webcam_pixels.cols, imgData.webcam_pixels.rows, imgData.webcam_pixels.data);

    if (!imgData.faces.empty()) {
      lastBackgroundUpdateTime = 0.f;
    }

    if(!backgroundTextureIsSet || lastBackgroundUpdateTime >= 15000.f) {
      std::cout << "set background\n";

      backgroundTexture.setImage(imgData.webcam_pixels.cols, imgData.webcam_pixels.rows, imgData.webcam_pixels.data);

      backgroundTextureIsSet = true;
      lastBackgroundUpdateTime = 0.f;
    }

    if (!imgData.faces.empty() && reactionState == ReactionState::Inactive && standbyBlitTimeout <= 0.f) {

      {
        auto rect = imgData.faces[0];

        auto rectTl = glm::vec2(rect.tl().x, rect.tl().y);
        auto rectBr = glm::vec2(rect.br().x, rect.br().y);
        glm::vec2 faceMin = rectTl / glm::vec2(imageProvider.width(), imageProvider.height());
        glm::vec2 faceMax = rectBr / glm::vec2(imageProvider.width(), imageProvider.height());

        glm::vec2 targetSize = glm::vec2(std::uniform_real_distribution<float>(.1f, .3f)(random),
                                          std::uniform_real_distribution<float>(.1f, .3f)(random));
        glm::vec2 targetMin = glm::vec2(std::uniform_real_distribution<float>(0.2f, 0.8f - targetSize.x)(random),
                                          std::uniform_real_distribution<float>(0.2f, 0.8f - targetSize.y)(random));
        glm::vec2 targetMax = targetMin + targetSize;

        faceBlitter.blit(webcamTexture, faceMin, faceMax, overlayFramebuffer, targetMin, targetMax);
      }

      ++standbyBlitCount;
      standbyBlitTimeout = std::uniform_real_distribution<float>(250.f, 750.f)(random);

      if(standbyBlitCount == 10u) {
        standbyBlitCount = 0u;

        standbyParticleRenderer.getClock().disableLooping();

        reactionState = ReactionState::FinishStandbyTimeline;
      }
    }
  }

  if (reactionState == ReactionState::FinishStandbyTimeline
      && standbyParticleRenderer.getClock().isPaused()) {

    std::cout << "start reaction\n";

    randomizeTimeline(reactionParticleRenderer.getTimeline(), random);
    reactionParticleRenderer.getClock().play();
    reactionParticleRenderer.enableSound(particleRendererGlobalState);

    reactionState = ReactionState::RenderReactionTimeline;
  }

  if (reactionState == ReactionState::RenderReactionTimeline
      && reactionParticleRenderer.getClock().isPaused()) {

    std::cout << "end reaction\n";

    overlayFramebuffer.clear();

    standbyParticleRenderer.getClock().enableLooping();
    standbyParticleRenderer.getClock().play();

    reactionState = ReactionState::Inactive;
  }

  standbyParticleRenderer.update(particleRendererGlobalState, dt);
  reactionParticleRenderer.update(particleRendererGlobalState, dt);
}

void Application::render() {
  // compose webcam and overlay into particleFramebuffer
  {
    particleFramebuffer.bind();
    glViewport(0, 0, particleFramebuffer.getWidth(), particleFramebuffer.getHeight());

    overlayComposePilpeline.bind();
    webcamTexture.bind(0);
    glUniform1i(overlayComposePilpeline_webcam_location, 0);
    overlayFramebuffer.getTexture().bind(1);
    glUniform1i(overlayComposePilpeline_overlay_location, 1);

    float overlayVisibility = 1.f;
    if(reactionState == ReactionState::RenderReactionTimeline) {
      if(reactionParticleRenderer.getClock().isPaused()) {
        overlayVisibility = 0.f;
      } else {
        auto p = reactionParticleRenderer.getClock().getPeriod();
        auto t = reactionParticleRenderer.getClock().getTime();
        overlayVisibility = std::min((p - t) / 2000.f, 1.f);
      }
    }
    glUniform1f(overlayComposePilpeline_overlayVisibility_location, overlayVisibility);

    screenRectBuffer.draw();
  }

  RendererParameters parameters(&particleFramebuffer.getTexture(), &backgroundTexture,
                                screen_width, screen_height,
                                imageProvider.width(), imageProvider.height());

  if (reactionState == ReactionState::RenderReactionTimeline) {
#if 0
    if(reactionParticleRenderer.getClock().getTime() < 100) {
      particleFramebuffer.getTexture().dbgSaveToFile("comp.bmp");
    }
#endif
    reactionParticleRenderer.render(particleRendererGlobalState, parameters);
  } else {
    standbyParticleRenderer.render(particleRendererGlobalState, parameters);
  }

  {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      printf("opengl error: %d\n", error);
  }
}
