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

  backgroundLoop.loadFromFile(resources, "sound/DroneLoopStereo01.wav");
  soundRenderer->play(&backgroundLoop,
                      sound::Renderer::PlayParameters().setLooping(true));

  whooshSamples.resize(5);
  whooshSamples[0].loadFromFile(resources, "sound/FXStereo01.wav");
  whooshSamples[1].loadFromFile(resources, "sound/FXStereo02.wav");
  whooshSamples[2].loadFromFile(resources, "sound/FXStereo03.wav");
  whooshSamples[3].loadFromFile(resources, "sound/FXStereo04.wav");
  whooshSamples[4].loadFromFile(resources, "sound/FXStereo05.wav");

  if (!webcam.open() || !webcam.getFrameSize(webcam_width, webcam_height)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam",
                             "Could not open webcam", NULL);
    return false;
  }

  for (auto i = 0; i < webcam_buffer.size; ++i) {
    auto &b = webcam_buffer.getBuffer(i);
    b.resize(webcam_width * webcam_height * 3);
    std::fill(b.begin(), b.end(), 0.f);
  }

  webcam_thread = std::thread([this] { this->webcamThreadFunc(); });

  particleRendererGlobalState.create();

  {
    auto timeline = std::make_unique<Timeline>(&effectRegistry);

    auto hueDisplace = timeline->emplaceEffectInstance<HueDisplaceEffect>();
    hueDisplace->timeBegin = 0.f;
    hueDisplace->timeEnd = 2000.f;
    hueDisplace->distance = .01f;
    hueDisplace->scaleByForegroundMask = 1.f;

    standbyParticleRenderer.setTimeline(std::move(timeline));
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

    reactionParticleRenderer.setTimeline(std::move(timeline));

    reactionParticleRenderer.getClock().disableLooping();
    reactionParticleRenderer.getClock().pause();
  }

  {
    auto testTimeline = std::make_unique<Timeline>(&effectRegistry);

    auto testJson = resources->readWholeTextFile("debug/particles.json");
    testTimeline->load(json::parse(testJson)["effects"]);

    testParticleRenderer.setTimeline(std::move(testTimeline));
  }

  current_frame_data.resize(webcam_width * webcam_height);
  particleBuffer.create(webcam_width * webcam_height);

  return true;
}

void Application::destroy() {
  kill_threads = true;

  particleBuffer.destroy();

  reactionParticleRenderer.reset();
  standbyParticleRenderer.reset();

  particleRendererGlobalState.destroy();

  if (webcam_thread.joinable())
    webcam_thread.join();

  webcam.close();

  soundRenderer->killAllVoices();
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
  auto frame = webcam_buffer.startCopyNew();
  if (frame) {
    // first frame is background plate
    // TODO: update this dynamically during runtime
    if (background_frame.empty()) {
      background_frame = *frame;
    }

    auto totalDifference = 0.f;

    for (size_t i = 0; i < current_frame_data.size(); ++i) {
      auto &particle = current_frame_data[i];
      auto x = i % webcam_width;
      auto y = i / webcam_width;
      particle.position[0] = x / (float)webcam_width;
      particle.position[1] = y / (float)webcam_height;
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

    if (totalDifference / (webcam_width * webcam_height) > .05f) {
      if (reactionState == ReactionState::Inactive) {
        std::cout << "trigger\n";

        standbyParticleRenderer.getClock().disableLooping();

        reactionState = ReactionState::FinishStandbyTimeline;
      }
    }

    webcam_buffer.finishCopy();
  }

  if (reactionState == ReactionState::FinishStandbyTimeline
      && standbyParticleRenderer.getClock().isPaused()) {

    std::cout << "start reaction\n";

    randomizeTimeline(reactionParticleRenderer.getTimeline(), random);
    reactionParticleRenderer.getClock().play();

    {
      std::uniform_int_distribution<> dis(0, whooshSamples.size() - 1);
      soundRenderer->play(
          &whooshSamples[dis(random)],
          sound::Renderer::PlayParameters().setStartDelay(0.));
      soundRenderer->play(
          &whooshSamples[dis(random)],
          sound::Renderer::PlayParameters().setStartDelay(1500.));
    }

    reactionState = ReactionState::RenderReactionTimeline;
  }

  if (reactionState == ReactionState::RenderReactionTimeline
      && reactionParticleRenderer.getClock().isPaused()) {

    std::cout << "end reaction\n";
    standbyParticleRenderer.getClock().enableLooping();
    standbyParticleRenderer.getClock().play();

    reactionState = ReactionState::Inactive;
  }

  standbyParticleRenderer.update(dt);
  reactionParticleRenderer.update(dt);

  testParticleRenderer.update(dt);
}

void Application::render() {
  graphics::Framebuffer::unbind();

  glClear(GL_COLOR_BUFFER_BIT);

  RendererParameters parameters(particleBuffer, random,
                                screen_width, screen_height,
                                webcam_width, webcam_height);

#if 1
  if (reactionState == ReactionState::RenderReactionTimeline) {
    reactionParticleRenderer.render(particleRendererGlobalState, parameters);
  } else {
    standbyParticleRenderer.render(particleRendererGlobalState, parameters);
  }
#else
  testParticleRenderer.render(particleRendererGlobalState, parameters);
#endif

  {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      printf("opengl error: %d\n", error);
  }
}

void Application::webcamThreadFunc() {
  while (!kill_threads) {
    auto frame = webcam_buffer.startWrite();
    if (!webcam.getFrame(frame->data())) {
      std::cerr << "webcam lost, you need to restart the app\n";
      break;
    }
    webcam_buffer.finishWrite();
  }
}
