#include "main.hpp"

#include "Application.hpp"

#include "Resources.hpp"
#include "effects/AllEffects.hpp"
#include "graphics/Window.hpp"
#include "sound/Renderer.hpp"

#include <stb_image_write.h>

constexpr uint32_t particles_width = 1280u, particles_height = 800u;
constexpr auto randomTrackIndex = 1u; // all other effects are on the default track (0)

#if WITH_EDIT_TOOLS
#define constexpr static
#endif

constexpr auto BACKGROUND_VISIBILITY = .2f;
constexpr auto BACKGROUND_FADE_TIME = 10000.f;

#if WITH_EDIT_TOOLS
#undef constexpr
#endif

bool Application::create(Resources *resources, graphics::Window *window,
                         sound::Renderer *soundRenderer) {
  this->window = window;
  this->soundRenderer = soundRenderer;

#ifdef NDEBUG
  random.seed(time(NULL));
#endif

  {
    const auto windowSize = window->getSize();
    screen_width = std::get<0>(windowSize);
    screen_height = std::get<1>(windowSize);
  }

#if WITH_EDIT_TOOLS
  if(!TwInit(TW_OPENGL_CORE, NULL)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not init AntTweakBar", "Could not init AntTweakBar", NULL);
    return false;
  }
  TwWindowSize(screen_width, screen_height);
#endif

  effectRegistry.registerEffect<BackgroundDifferenceGlowEffect>();
  effectRegistry.registerEffect<ConvergeCircle2Effect>();
  effectRegistry.registerEffect<ConvergeCircleEffect>();
  effectRegistry.registerEffect<ConvergePoint2Effect>();
  effectRegistry.registerEffect<ConvergePointEffect>();
  effectRegistry.registerEffect<HueDisplace2Effect>();
  effectRegistry.registerEffect<HueDisplaceEffect>();
  effectRegistry.registerEffect<ParticleDisplaceEffect>();
  effectRegistry.registerEffect<ParticleSizeByHueEffect>();
  effectRegistry.registerEffect<ParticleSizeModifyEffect>();
  effectRegistry.registerEffect<ParticleSpacingEffect>();
  effectRegistry.registerEffect<ReduceParticleCountEffect>();
  effectRegistry.registerEffect<SmearEffect>();
  effectRegistry.registerEffect<SmoothTrailsEffect>();
  effectRegistry.registerEffect<StandingWaveEffect>();
  effectRegistry.registerEffect<TrailsEffect>();
  effectRegistry.registerEffect<VignetteEffect>();
  effectRegistry.registerEffect<WaveEffect>();

  if(!sampleLibrary.create(random, resources)) {
    return false;
  }

  soundRenderer->play(sampleLibrary.getBackgroundLoop(),
                      sound::Renderer::PlayParameters().setLooping(true));

  if(!imageProvider.create(resources) || !imageProvider.start()) {
    return false;
  }

  screenRectBuffer.create();

  webcamImageTransform.create(&screenRectBuffer, imageProvider.width(), imageProvider.height(), particles_width, particles_height);
  faceBlitter.create(&screenRectBuffer, particles_width, particles_height);

  overlayCompose.create(&screenRectBuffer);

  particleRendererGlobalState.create(soundRenderer, &sampleLibrary, &random, &screenRectBuffer, screen_width, screen_height);
  backgroundParticleRendererGlobalState.create(soundRenderer, &sampleLibrary, &random, &screenRectBuffer, screen_width, screen_height);

  {
    auto timeline = std::make_unique<Timeline>(&effectRegistry);

    auto hueDisplace = timeline->emplaceEffectInstance<HueDisplaceEffect>();
    hueDisplace->timeBegin = 0.f;
    hueDisplace->timeEnd = 4000.f;
    hueDisplace->distance = .1f;
    hueDisplace->scaleByForegroundMask = 1.f;

    standbyParticleRenderer.setTimeline(particleRendererGlobalState, std::move(timeline));
  }

  {
    auto timeline = reactionTimelineRandomizer.createTimeline(&effectRegistry);
    reactionParticleRenderer.setTimeline(particleRendererGlobalState, std::move(timeline));

    reactionParticleRenderer.getClock().disableLooping();
    reactionParticleRenderer.getClock().pause();
  }

  {
    auto timeline = std::make_unique<Timeline>(&effectRegistry);

    auto hueDisplace = timeline->emplaceEffectInstance<HueDisplaceEffect>();
    hueDisplace->timeBegin = 0.f;
    hueDisplace->timeEnd = 4000.f;
    hueDisplace->distance = .1f;
    hueDisplace->scaleByForegroundMask = 1.f;

    auto glow = timeline->emplaceEffectInstance<BackgroundDifferenceGlowEffect>();
    glow->timeBegin = hueDisplace->timeBegin;
    glow->timeEnd = hueDisplace->timeEnd;

    backgroundParticleRenderer.setTimeline(backgroundParticleRendererGlobalState, std::move(timeline));

    backgroundParticleRenderer.getClock().play();
  }

  webcamInputTexture.create(imageProvider.width(), imageProvider.height());
  webcamFramebuffer.create(particles_width, particles_height);
  backgroundFramebuffer.create(particles_width, particles_height);
  particleSourceFramebuffer.create(particles_width, particles_height);
  particleOutputFramebuffer.create(screen_width, screen_height);
  backgroundParticleOutputFramebuffer.create(screen_width, screen_height);

  finalComposite.create(&screenRectBuffer);

  // prevent undefined images at startup
  webcamFramebuffer.clear();
  backgroundFramebuffer.clear();

#if WITH_EDIT_TOOLS
  {
    auto bar = TwNewBar("global");
    TwDefine("global position='0 0' size='200 1000' refresh=0.05");

    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, &paused, "label=paused");
    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, &skipStandby, "label='skip standby'");

    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, standbyParticleRenderer.getClock().editGetTimeP(), "group=standby label=time");
    TwAddVarRO(bar, NULL, TW_TYPE_FLOAT, standbyParticleRenderer.getClock().editGetPeriodP(), "group=standby label=period");
    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, standbyParticleRenderer.getClock().editGetPausedP(), "group=standby label=paused");

    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, reactionParticleRenderer.getClock().editGetTimeP(), "group=reaction label=time");
    TwAddVarRO(bar, NULL, TW_TYPE_FLOAT, reactionParticleRenderer.getClock().editGetPeriodP(), "group=reaction label=period");
    TwAddVarRW(bar, NULL, TW_TYPE_BOOLCPP, reactionParticleRenderer.getClock().editGetPausedP(), "group=reaction label=paused");

    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, webcamImageTransform.editGetBrightnessMulP(), "group='webcam image' label='brightness *'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, webcamImageTransform.editGetBrightnessAddP(), "group='webcam image' label='brightness +'");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, webcamImageTransform.editGetSaturationP(), "group='webcam image' label=saturation");

    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &BACKGROUND_VISIBILITY, "group='background' label=visibility");
    TwAddVarRW(bar, NULL, TW_TYPE_FLOAT, &BACKGROUND_FADE_TIME, "group='background' label='fade time'");
  }
#endif

  return true;
}

void Application::destroy() {
  finalComposite.destroy();

  backgroundParticleOutputFramebuffer.destroy();
  particleOutputFramebuffer.destroy();
  particleSourceFramebuffer.destroy();
  backgroundFramebuffer.destroy();
  webcamFramebuffer.destroy();
  webcamInputTexture.destroy();

  reactionParticleRenderer.reset();
  standbyParticleRenderer.reset();

  particleRendererGlobalState.destroy();
  backgroundParticleRendererGlobalState.destroy();

  overlayCompose.destroy();

  faceBlitter.destroy();
  webcamImageTransform.destroy();

  screenRectBuffer.destroy();

  imageProvider.stop();
  imageProvider.destroy();

  soundRenderer->killAllVoices();

  sampleLibrary.destroy();

#if WITH_EDIT_TOOLS
  TwTerminate();
#endif
}

void Application::reshape(uint32_t width, uint32_t height) {
  screen_width = width;
  screen_height = height;

  particleOutputFramebuffer.resize(width, height);
  backgroundParticleOutputFramebuffer.resize(width, height);
  particleRendererGlobalState.resize(particleOutputFramebuffer.getWidth(), particleOutputFramebuffer.getHeight());
  backgroundParticleRendererGlobalState.resize(backgroundParticleOutputFramebuffer.getWidth(), backgroundParticleOutputFramebuffer.getHeight());
}

bool Application::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
#if WITH_EDIT_TOOLS
    if(TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION)) continue;
#endif

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
      case SDL_SCANCODE_G:
      soundRenderer->glitch(1000.f, 1000.f, 50.f);
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
      case SDL_SCANCODE_S:
      // save current reaction timeline
      {
        json j;
        j["period"] = reactionParticleRenderer.getTimeline()->getPeriod();
        reactionParticleRenderer.getTimeline()->save(j["effects"]);

        std::ofstream f("reaction.json");
        f << j;
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

static glm::vec2 sampleCircle(std::default_random_engine &random) {
  float a = std::uniform_real_distribution<float>(0.f, 2.f * PI)(random);
  float r = std::sqrt(std::uniform_real_distribution<float>(.5f, 1.f)(random));
  return glm::vec2(r * std::cos(a), r * std::sin(a));
}

void Application::update(float dt) {
#if WITH_EDIT_TOOLS
  reactionTimelineRandomizer.editUpdate();
  if(paused) return;
#endif

  if(reactionState == ReactionState::Inactive || reactionState == ReactionState::FinishStandbyTimeline) {
    standbyBlitTimeout -= dt;
  }

  auto imgDataP = imageProvider.consume();
  if (imgDataP) {
    auto &imgData = *imgDataP;

    webcamInputTexture.setImage(imgData.webcam_pixels.cols, imgData.webcam_pixels.rows, imgData.webcam_pixels.data);

    std::swap(webcamFramebuffer, backgroundFramebuffer);
    webcamImageTransform.draw(webcamInputTexture, webcamFramebuffer);

    if (!imgData.faces.empty()
      && (reactionState == ReactionState::Inactive || reactionState == ReactionState::FinishStandbyTimeline)
      && standbyBlitTimeout <= 0.f) {

      {
        auto rect = imgData.faces[std::uniform_int_distribution<size_t>(0u, imgData.faces.size()-1u)(random)];

        auto rectTl = glm::vec2(rect.tl().x, rect.tl().y);
        auto rectBr = glm::vec2(rect.br().x, rect.br().y);
        glm::vec2 face1 = glm::vec2(webcamImageTransform.getInverseTransform() * glm::vec3(rectTl / glm::vec2(imageProvider.width(), imageProvider.height()), 1.f));
        glm::vec2 face2 = glm::vec2(webcamImageTransform.getInverseTransform() * glm::vec3(rectBr / glm::vec2(imageProvider.width(), imageProvider.height()), 1.f));

        glm::vec2 faceMin = glm::min(face1, face2);
        glm::vec2 faceMax = glm::max(face1, face2);

        glm::vec2 targetSize = glm::vec2(std::uniform_real_distribution<float>(.1f, .2f)(random),
                                          std::uniform_real_distribution<float>(.1f, .2f)(random));
        glm::vec2 targetCenter = sampleCircle(random) * glm::vec2(.4f) + glm::vec2(.5f);
        glm::vec2 targetMin = targetCenter - targetSize / 2.f;
        glm::vec2 targetMax = targetCenter + targetSize / 2.f;

        faceBlitter.blit(webcamFramebuffer.getTexture(), faceMin, faceMax, targetMin, targetMax, backgroundFramebuffer.getTexture());
      }

      standbyBlitTimeout = std::normal_distribution<float>(250.f, 100.f)(random);

      if(reactionState == ReactionState::Inactive) {
        ++standbyBlitCount;
      }

      if(standbyBlitCount == standbyBlitTargetCount) {
        standbyBlitCount = 0u;
        standbyBlitTargetCount = std::uniform_int_distribution<uint32_t>(40u, 50u)(random);

        standbyParticleRenderer.getClock().disableLooping();

        reactionState = ReactionState::FinishStandbyTimeline;
      }
    }
  }

  if (reactionState == ReactionState::FinishStandbyTimeline
      && standbyParticleRenderer.getClock().isPaused()) {

    std::cout << "start reaction\n";

    auto result = reactionTimelineRandomizer.randomize(random);
    reactionParticleRenderer.refreshPeriod();
    reactionParticleRenderer.getClock().play();
    reactionParticleRenderer.enableSound(particleRendererGlobalState);

    soundRenderer->play(sampleLibrary.getSample("intro01"), sound::Renderer::PlayParameters().setStartDelay(result.fadeInBegin));
    soundRenderer->play(sampleLibrary.getSample("outro01"), sound::Renderer::PlayParameters().setStartDelay(result.fadeOutBegin));

    if(result.glitchLength > 0.f) {
      soundRenderer->glitch(result.glitchBegin, result.glitchLength, std::uniform_real_distribution<double>(25., 80.)(random));
      //soundRenderer->play(sampleLibrary.getSample("glitch"), sound::Renderer::PlayParameters().setStartDelay(result.glitchBegin));
    }

    reactionState = ReactionState::RenderReactionTimeline;
  }

  if (reactionState == ReactionState::RenderReactionTimeline
      && reactionParticleRenderer.getClock().isPaused()) {

    std::cout << "end reaction\n";

    faceBlitter.clear();

    standbyParticleRenderer.getClock().enableLooping();
    standbyParticleRenderer.getClock().play();

#if WITH_EDIT_TOOLS
    *reactionParticleRenderer.getClock().editGetTimeP() = 0.f;
    if(skipStandby) {
      standbyParticleRenderer.getClock().pause();
      reactionState = ReactionState::FinishStandbyTimeline;
    } else
#endif
    reactionState = ReactionState::Inactive;
  }

  faceBlitter.update(dt);

  standbyParticleRenderer.update(particleRendererGlobalState, dt);
  reactionParticleRenderer.update(particleRendererGlobalState, dt);
  backgroundParticleRenderer.update(backgroundParticleRendererGlobalState, dt);
}

void Application::render() {
  faceBlitter.draw();

  // compose webcam and overlay into particleSourceFramebuffer
  {
    float overlayVisibility = 1.f;
    if(reactionState == ReactionState::RenderReactionTimeline) {
      if(reactionParticleRenderer.getClock().isPaused()) {
        overlayVisibility = 0.f;
      } else {
        auto p = reactionParticleRenderer.getClock().getPeriod() - 1000.f;
        auto t = reactionParticleRenderer.getClock().getTime();
        overlayVisibility = std::min(std::max((p - t) / 4000.f, 0.f), 1.f);
      }
    }

    overlayCompose.draw(webcamFramebuffer.getTexture(), faceBlitter.getResultTexture(), overlayVisibility, particleSourceFramebuffer);
  }

  RendererParameters parameters(&particleSourceFramebuffer.getTexture(), &backgroundFramebuffer.getTexture(),
                                &particleOutputFramebuffer);

  if (reactionState == ReactionState::RenderReactionTimeline) {
#if 0
    if(reactionParticleRenderer.getClock().getTime() < 100) {
      particleSourceFramebuffer.getTexture().dbgSaveToFile("comp.bmp");
    }
#endif
    reactionParticleRenderer.render(particleRendererGlobalState, parameters);

    {
      RendererParameters parameters(&particleSourceFramebuffer.getTexture(), &backgroundFramebuffer.getTexture(),
                                &backgroundParticleOutputFramebuffer);
      backgroundParticleRenderer.render(particleRendererGlobalState, parameters);
    }
  } else {
    standbyParticleRenderer.render(particleRendererGlobalState, parameters);
  }

  {
    auto backgroundVisibility = 0.f;

    if(reactionState == ReactionState::RenderReactionTimeline) {
      const auto time = reactionParticleRenderer.getClock().getTime();
      const auto period = reactionParticleRenderer.getClock().getPeriod();
      backgroundVisibility = glm::clamp(glm::min(time, (period - time)) / BACKGROUND_FADE_TIME, 0.f, 1.f) * BACKGROUND_VISIBILITY;
    }

    finalComposite.draw(particleOutputFramebuffer.getTexture(), backgroundParticleOutputFramebuffer.getTexture(), backgroundVisibility, screen_width, screen_height);
  }

  {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      printf("opengl error: %d\n", error);
  }

#if WITH_EDIT_TOOLS
  TwDraw();
#endif
}
