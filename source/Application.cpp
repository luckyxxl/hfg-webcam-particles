#include "main.hpp"

#include "Application.hpp"

#include "Resources.hpp"
#include "effects/ConvergeCircleEffect.hpp"
#include "effects/ConvergePointEffect.hpp"
#include "effects/HueDisplaceEffect.hpp"
#include "effects/SmearEffect.hpp"
#include "effects/SmoothTrailsEffect.hpp"
#include "effects/TrailsEffect.hpp"
#include "effects/WaveEffect.hpp"
#include "graphics/Window.hpp"
#include "sound/Renderer.hpp"

bool Application::create(Resources *resources, graphics::Window *window,
                         sound::Renderer *soundRenderer) {
  this->window = window;
  this->soundRenderer = soundRenderer;

  effectRegistry.registerEffect<ConvergeCircleEffect>();
  effectRegistry.registerEffect<ConvergePointEffect>();
  effectRegistry.registerEffect<HueDisplaceEffect>();
  effectRegistry.registerEffect<SmearEffect>();
  effectRegistry.registerEffect<SmoothTrailsEffect>();
  effectRegistry.registerEffect<TrailsEffect>();
  effectRegistry.registerEffect<WaveEffect>();

  if(!sampleLibrary.create(random, resources)) {
    return false;
  }

  soundRenderer->play(sampleLibrary.getBackgroundLoop(),
                      sound::Renderer::PlayParameters().setLooping(true));

  if(!imageProvider.start()) {
    return false;
  }

  particleRendererGlobalState.create(soundRenderer, &sampleLibrary, &random);

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

  current_frame_data.resize(imageProvider.size());
  particleBuffer.create(imageProvider.size());

  return true;
}

void Application::destroy() {
  imageProvider.stop();

  particleBuffer.destroy();

  reactionParticleRenderer.reset();
  standbyParticleRenderer.reset();

  particleRendererGlobalState.destroy();

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

static void rgb2Hsv(float *hsv, const float *rgb) {
  const auto cMax = std::max(std::max(rgb[0], rgb[1]), rgb[2]);
  const auto cMin = std::min(std::min(rgb[0], rgb[1]), rgb[2]);
  const auto d = cMax - cMin;

  if (d < 0.00001f || cMax < 0.00001f) {
    hsv[0] = 0.f;
    hsv[1] = 0.f;
    hsv[2] = cMax;
    return;
  }

  float h;
  if (cMax == rgb[0]) {
    h = (rgb[1] - rgb[2]) / d;
    if (h < 0)
      h += 6.f;
  } else if (cMax == rgb[1]) {
    h = (rgb[2] - rgb[0]) / d + 2.f;
  } else {
    h = (rgb[0] - rgb[1]) / d + 4.f;
  }

  hsv[0] = h * 60.f * PI / 180.f;
  hsv[1] = d / cMax;
  hsv[2] = cMax;
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
  ImageData imgData;
  imageProvider >> imgData;
  if (!imgData.empty()) {
    auto *frame = &imgData.webcam_pixels;
    // first frame is background plate
    // TODO: update this dynamically during runtime
    if (background_frame.empty()) {
      background_frame = *frame;
    }

    auto totalDifference = 0.f;

    for (size_t i = 0; i < current_frame_data.size(); ++i) {
      auto &particle = current_frame_data[i];
      auto x = i % imageProvider.width();
      auto y = i / imageProvider.width();
      particle.position[0] = x / (float)imageProvider.width();
      particle.position[1] = y / (float)imageProvider.height();
      particle.rgb[0] = (*frame)[i * 3 + 0];
      particle.rgb[1] = (*frame)[i * 3 + 1];
      particle.rgb[2] = (*frame)[i * 3 + 2];
      rgb2Hsv(particle.hsv, particle.rgb);

      const auto backgroundDifference =
          ((*frame)[i * 3 + 0] - background_frame[i * 3 + 0]) *
              ((*frame)[i * 3 + 0] - background_frame[i * 3 + 0]) +
          ((*frame)[i * 3 + 1] - background_frame[i * 3 + 1]) *
              ((*frame)[i * 3 + 1] - background_frame[i * 3 + 1]) +
          ((*frame)[i * 3 + 2] - background_frame[i * 3 + 2]) *
              ((*frame)[i * 3 + 2] - background_frame[i * 3 + 2]);

      totalDifference += backgroundDifference;

      // TODO: animate this
      particle.foregroundMask =
          std::min(std::max(backgroundDifference - .2f, 0.f) * 100.f, 1.f);
    }

    particleBuffer.setParticleData(current_frame_data.data(),
                                   current_frame_data.size());

    if (totalDifference / imageProvider.size() > .05f) {
      if (reactionState == ReactionState::Inactive) {
        std::cout << "trigger\n";

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

    standbyParticleRenderer.getClock().enableLooping();
    standbyParticleRenderer.getClock().play();

    reactionState = ReactionState::Inactive;
  }

  standbyParticleRenderer.update(particleRendererGlobalState, dt);
  reactionParticleRenderer.update(particleRendererGlobalState, dt);
}

void Application::render() {
  graphics::Framebuffer::unbind();

  glClear(GL_COLOR_BUFFER_BIT);

  RendererParameters parameters(particleBuffer,
                                screen_width, screen_height,
                                imageProvider.width(), imageProvider.height());

  if (reactionState == ReactionState::RenderReactionTimeline) {
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
