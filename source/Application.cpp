#include "main.hpp"

#include "Application.hpp"

#include "Resources.hpp"
#include "effects/ConvergeCircleEffect.hpp"
#include "effects/ConvergePointEffect.hpp"
#include "effects/HueDisplaceEffect.hpp"
#include "effects/ParticleDisplaceEffect.hpp"
#include "effects/ParticleSizeByHueEffect.hpp"
#include "effects/ParticleSpacingEffect.hpp"
#include "effects/SmearEffect.hpp"
#include "effects/SmoothTrailsEffect.hpp"
#include "effects/StandingWaveEffect.hpp"
#include "effects/TrailsEffect.hpp"
#include "effects/WaveEffect.hpp"
#include "graphics/Window.hpp"
#include "sound/Renderer.hpp"
#include "IntervalMath.hpp"

#include <stb_image_write.h>

constexpr uint32_t particles_width = 320u, particles_height = 200u;

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

  effectRegistry.registerEffect<ConvergeCircleEffect>();
  effectRegistry.registerEffect<ConvergePointEffect>();
  effectRegistry.registerEffect<HueDisplaceEffect>();
  effectRegistry.registerEffect<ParticleDisplaceEffect>();
  effectRegistry.registerEffect<ParticleSizeByHueEffect>();
  effectRegistry.registerEffect<ParticleSpacingEffect>();
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

  if(!imageProvider.create(resources) || !imageProvider.start()) {
    return false;
  }

  screenRectBuffer.create();

  webcamImageTransform.create(&screenRectBuffer, imageProvider.width(), imageProvider.height(), particles_width, particles_height);
  faceBlitter.create(&screenRectBuffer, particles_width, particles_height);

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

  particleRendererGlobalState.create(soundRenderer, &sampleLibrary, &random, &screenRectBuffer, screen_width, screen_height);

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
    auto timeline = std::make_unique<Timeline>(&effectRegistry);

    timeline->emplaceEffectInstance<ConvergeCircleEffect>();
    timeline->emplaceEffectInstance<ConvergePointEffect>();
    timeline->emplaceEffectInstance<ParticleDisplaceEffect>();
    timeline->emplaceEffectInstance<HueDisplaceEffect>();
    timeline->emplaceEffectInstance<ParticleSpacingEffect>();
    timeline->emplaceEffectInstance<StandingWaveEffect>();
    timeline->emplaceEffectInstance<WaveEffect>();

    auto accum = timeline->emplaceEffectInstance<TrailsEffect>();
    accum->fadeIn = 1000.f;
    accum->fadeOut = 1000.f;
    accum->strength = .8f;

    reactionParticleRenderer.setTimeline(particleRendererGlobalState, std::move(timeline));

    reactionParticleRenderer.getClock().disableLooping();
    reactionParticleRenderer.getClock().pause();
  }

  webcamInputTexture.create(imageProvider.width(), imageProvider.height());
  webcamFramebuffer.create(particles_width, particles_height);
  backgroundFramebuffer.create(particles_width, particles_height);
  particleSourceFramebuffer.create(particles_width, particles_height);
  particleOutputFramebuffer.create(screen_width, screen_height);

  finalComposite.create(&screenRectBuffer);

  // prevent undefined images at startup
  webcamFramebuffer.clear();
  backgroundFramebuffer.clear();

  return true;
}

void Application::destroy() {
  finalComposite.destroy();

  particleOutputFramebuffer.destroy();
  particleSourceFramebuffer.destroy();
  backgroundFramebuffer.destroy();
  webcamFramebuffer.destroy();
  webcamInputTexture.destroy();

  reactionParticleRenderer.reset();
  standbyParticleRenderer.reset();

  particleRendererGlobalState.destroy();

  overlayComposePilpeline.destroy();

  faceBlitter.destroy();
  webcamImageTransform.destroy();

  screenRectBuffer.destroy();

  imageProvider.stop();
  imageProvider.destroy();

  soundRenderer->killAllVoices();

  sampleLibrary.destroy();
}

void Application::reshape(uint32_t width, uint32_t height) {
  screen_width = width;
  screen_height = height;

  particleOutputFramebuffer.resize(width, height);
  particleRendererGlobalState.resize(particleOutputFramebuffer.getWidth(),
    particleOutputFramebuffer.getHeight());
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

static void removeEmptySpace(Timeline *timeline) {
  const auto instanceCount = timeline->getInstanceCount();
  if(instanceCount == 0u) return;

  std::vector<Interval> intervals;
  intervals.reserve(instanceCount);
  timeline->forEachInstance([&](const IEffect &i) {
    if(!i.enabled || i.isAccumulationEffect()
        || !strcmp(i.getName(), "ParticleDisplaceEffect")
        || !strcmp(i.getName(), "ParticleSpacingEffect")) return;

    intervals.emplace_back(i.timeBegin, i.timeEnd);
  });
  std::sort(intervals.begin(), intervals.end());

  auto emptyIntervals = getEmptyIntervals(intervals, 0.f);

  assert(std::is_sorted(emptyIntervals.begin(), emptyIntervals.end()));

  for(auto interval = emptyIntervals.begin(); interval != emptyIntervals.end(); ++interval) {
    const auto move = -interval->length();

    timeline->forEachInstance([&](IEffect &i) {
      if(i.isAccumulationEffect()) return;

      if(interval->start() <= i.timeBegin) {
        i.timeBegin += move;
        i.timeEnd += move;
      }
    });

    for(auto i = interval+1; i != emptyIntervals.end(); ++i) {
      i->start() += move;
      i->end() += move;
    }
  }
}

static void randomizeTimeline(Timeline *timeline,
                              std::default_random_engine &random) {
  const auto period = std::uniform_real_distribution<float>(10000.f, 30000.f)(random);
  const auto minLength = 3000.f;

  timeline->forEachInstance([&](IEffect &i) {
    if(i.isAccumulationEffect()) {
      i.timeBegin = 0.f;
      i.timeEnd = 0.f;
    } else {
      i.timeBegin = std::uniform_real_distribution<float>(0.f, period - minLength)
                      (random);
      i.timeEnd = i.timeBegin + std::uniform_real_distribution<float>(minLength,
                                  period - i.timeBegin)(random);
      i.randomizeConfig(random);
    }
  });

  {
    size_t enabledCount;
    do {
      enabledCount = 0u;
      timeline->forEachInstance([&](IEffect &i) {
        i.enabled = std::bernoulli_distribution(.95)(random);
        if(i.enabled) ++enabledCount;
      });
    } while(enabledCount < 2u);
  }

  removeEmptySpace(timeline);

  timeline->forEachInstance([&](IEffect &i) {
    if(i.isAccumulationEffect()) {
      i.timeEnd = timeline->getPeriod();
    }
  });
}

static glm::vec2 sampleCircle(std::default_random_engine &random) {
  float a = std::uniform_real_distribution<float>(0.f, 2.f * PI)(random);
  float r = std::sqrt(std::uniform_real_distribution<float>(.5f, 1.f)(random));
  return glm::vec2(r * std::cos(a), r * std::sin(a));
}

void Application::update(float dt) {
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
        glm::vec2 face1 = glm::vec2(webcamImageTransform.getTransform() * glm::vec3(rectTl / glm::vec2(imageProvider.width(), imageProvider.height()), 1.f));
        glm::vec2 face2 = glm::vec2(webcamImageTransform.getTransform() * glm::vec3(rectBr / glm::vec2(imageProvider.width(), imageProvider.height()), 1.f));

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
        standbyBlitTargetCount = std::uniform_int_distribution<uint32_t>(32u, 128u)(random);

        standbyParticleRenderer.getClock().disableLooping();

        reactionState = ReactionState::FinishStandbyTimeline;
      }
    }
  }

  if (reactionState == ReactionState::FinishStandbyTimeline
      && standbyParticleRenderer.getClock().isPaused()) {

    std::cout << "start reaction\n";

    randomizeTimeline(reactionParticleRenderer.getTimeline(), random);
    reactionParticleRenderer.refreshPeriod();
    reactionParticleRenderer.getClock().play();
    reactionParticleRenderer.enableSound(particleRendererGlobalState);

    reactionState = ReactionState::RenderReactionTimeline;
  }

  if (reactionState == ReactionState::RenderReactionTimeline
      && reactionParticleRenderer.getClock().isPaused()) {

    std::cout << "end reaction\n";

    faceBlitter.clear();

    standbyParticleRenderer.getClock().enableLooping();
    standbyParticleRenderer.getClock().play();

    reactionState = ReactionState::Inactive;
  }

  faceBlitter.update(dt);

  standbyParticleRenderer.update(particleRendererGlobalState, dt);
  reactionParticleRenderer.update(particleRendererGlobalState, dt);
}

void Application::render() {
  faceBlitter.draw();

  // compose webcam and overlay into particleSourceFramebuffer
  {
    particleSourceFramebuffer.bind();
    glViewport(0, 0, particleSourceFramebuffer.getWidth(), particleSourceFramebuffer.getHeight());

    overlayComposePilpeline.bind();
    webcamFramebuffer.getTexture().bind(0);
    glUniform1i(overlayComposePilpeline_webcam_location, 0);
    faceBlitter.getResultTexture().bind(1);
    glUniform1i(overlayComposePilpeline_overlay_location, 1);

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
    glUniform1f(overlayComposePilpeline_overlayVisibility_location, overlayVisibility);

    screenRectBuffer.draw();
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
  } else {
    standbyParticleRenderer.render(particleRendererGlobalState, parameters);
  }

  finalComposite.draw(particleOutputFramebuffer.getTexture(), screen_width, screen_height);

  {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      printf("opengl error: %d\n", error);
  }
}
