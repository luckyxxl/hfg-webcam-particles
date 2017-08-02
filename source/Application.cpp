#include "main.hpp"

#include "Application.hpp"

#include "effects/HueDisplaceEffect.hpp"
#include "effects/ConvergeCircleEffect.hpp"

bool Application::create(Resources *resources, sound::Renderer *soundRenderer) {
  this->soundRenderer = soundRenderer;

  //testSample.loadFromFile(resources, "sound_debug/test.wav");

  //loopTestSample.loadFromFile(resources, "sound_debug/warning_beep.wav");
  //soundRenderer->play(&loopTestSample, sound::Renderer::PlayParameters().setLooping(true));

  backgroundLoop.loadFromFile(resources, "sound/DroneLoopStereo01.wav");
  soundRenderer->play(&backgroundLoop, sound::Renderer::PlayParameters().setLooping(true));

  whooshSamples.resize(5);
  whooshSamples[0].loadFromFile(resources, "sound/FXStereo01.wav");
  whooshSamples[1].loadFromFile(resources, "sound/FXStereo02.wav");
  whooshSamples[2].loadFromFile(resources, "sound/FXStereo03.wav");
  whooshSamples[3].loadFromFile(resources, "sound/FXStereo04.wav");
  whooshSamples[4].loadFromFile(resources, "sound/FXStereo05.wav");

  if(!webcam.open() || !webcam.getFrameSize(webcam_width, webcam_height)) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open webcam", "Could not open webcam", NULL);
    return false;
  }

  for(auto i=0; i<webcam_buffer.size; ++i) {
    auto &b = webcam_buffer.getBuffer(i);
    b.resize(webcam_width * webcam_height * 3);
    std::fill(b.begin(), b.end(), 0.f);
  }

  webcam_thread = std::thread([this] { this->webcamThreadFunc(); });

  {
    const auto vertexShaderSource = resources->readWholeTextFile("test.glslv");
    const auto fragmentShaderSource = resources->readWholeTextFile("test.glslf");
    pipeline.create(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
  }

  invImageAspectRatio_location = pipeline.getUniformLocation("invImageAspectRatio");
  invScreenAspectRatio_location = pipeline.getUniformLocation("invScreenAspectRatio");
  viewProjectionMatrix_location = pipeline.getUniformLocation("viewProjectionMatrix");
  invViewProjectionMatrix_location = pipeline.getUniformLocation("invViewProjectionMatrix");
  globalTime_location = pipeline.getUniformLocation("globalTime");
  globalEffectTime_location = pipeline.getUniformLocation("globalEffectTime");

  current_frame_data.resize(webcam_width * webcam_height);
  particleBuffer.create(webcam_width * webcam_height);

  effectRegistry.registerEffect<HueDisplaceEffect>();
  effectRegistry.registerEffect<ConvergeCircleEffect>();

  {
    Timeline testTimeline(&effectRegistry);

    auto testJson = resources->readWholeTextFile("debug/particles.json");
    testTimeline.load(json::parse(testJson)["effects"]);

    ShaderBuilder vertexShader, fragmentShader;

    unsigned instanceId = 0;
    testTimeline.forEachInstance([&](const IEffect &i) {
      Uniforms uniforms(instanceId);
      i.registerEffect(uniforms, vertexShader, fragmentShader);
      vertexShader.appendUniforms(uniforms);
      fragmentShader.appendUniforms(uniforms);
      ++instanceId;
    });

    std::cout << vertexShader.assemble() << "\n" /*<< fragmentShader.assemble()*/;

    json testSave;
    testTimeline.save(testSave);

    std::cout << testSave.dump(2) << "\n";
  }

  return true;
}

void Application::destroy() {
  kill_threads = true;

  particleBuffer.destroy();

  pipeline.destroy();

  if(webcam_thread.joinable()) webcam_thread.join();

  webcam.close();
}

void Application::reshape(uint32_t width, uint32_t height) {
  screen_width = width;
  screen_height = height;

  glViewport(0, 0, width, height);
}

void Application::handleEvent(const SDL_Event &event) {
  switch(event.type) {
    case SDL_KEYDOWN:
    switch(event.key.keysym.scancode) {
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
    break;
  }
}

static void rgb2Hsv(float *hsv, const float *rgb) {
  const auto cMax = std::max(std::max(rgb[0], rgb[1]), rgb[2]);
  const auto cMin = std::min(std::min(rgb[0], rgb[1]), rgb[2]);
  const auto d = cMax - cMin;

  if(d < 0.00001f || cMax < 0.00001f) {
    hsv[0] = 0.f; hsv[1] = 0.f; hsv[2] = cMax;
    return;
  }

  float h;
  if(cMax == rgb[0]) {
    h = (rgb[1] - rgb[2]) / d;
    if(h < 0) h += 6.f;
  } else if(cMax == rgb[1]) {
    h = (rgb[2] - rgb[0]) / d + 2.f;
  } else {
    h = (rgb[0] - rgb[1]) / d + 4.f;
  }

  hsv[0] = h * 60.f * PI / 180.f;
  hsv[1] = d / cMax;
  hsv[2] = cMax;
}

void Application::update(float dt) {
  auto frame = webcam_buffer.startCopyNew();
  if(frame) {
    // first frame is background plate
    //TODO: update this dynamically during runtime
    if(background_frame.empty()) {
      background_frame = *frame;
    }

    auto totalDifference = 0.f;

    for(size_t i=0; i<current_frame_data.size(); ++i) {
      auto &particle = current_frame_data[i];
      auto x = i % webcam_width;
      auto y = i / webcam_width;
      particle.position[0] = x / (float)webcam_width;
      particle.position[1] = y / (float)webcam_height;
      particle.rgb[0] = (*frame)[i*3+0];
      particle.rgb[1] = (*frame)[i*3+1];
      particle.rgb[2] = (*frame)[i*3+2];
      rgb2Hsv(particle.hsv, particle.rgb);

      const auto backgroundDifference =
        ((*frame)[i*3+0] - background_frame[i*3+0]) * ((*frame)[i*3+0] - background_frame[i*3+0]) +
        ((*frame)[i*3+1] - background_frame[i*3+1]) * ((*frame)[i*3+1] - background_frame[i*3+1]) +
        ((*frame)[i*3+2] - background_frame[i*3+2]) * ((*frame)[i*3+2] - background_frame[i*3+2]);

      totalDifference += backgroundDifference;

      //TODO: animate this
      particle.localEffectStrength = std::min(std::max(backgroundDifference - .2f, 0.f) * 100.f, 1.f);
    }

    particleBuffer.setParticleData(current_frame_data.data(), current_frame_data.size());

    if(totalDifference / (webcam_width * webcam_height) > .05f) {
      if(!globalEffectTimeoutActive) {
        std::cout << "trigger\n";
        globalEffectTimeoutActive = true;

        //soundRenderer->play(&testSample);
        //soundRenderer->play(&testSample, sound::Renderer::PlayParameters().setStartDelay(2.5));

        {
          std::uniform_int_distribution<> dis(0, whooshSamples.size()-1);
          soundRenderer->play(&whooshSamples[dis(random)], sound::Renderer::PlayParameters().setStartDelay(1.));
          soundRenderer->play(&whooshSamples[dis(random)], sound::Renderer::PlayParameters().setStartDelay(2.5));
        }
      }
    }

    webcam_buffer.finishCopy();
  }

  if(globalEffectTimeoutActive) {
    globalEffectTimeout -= dt;
    if(globalEffectTimeout <= 0.f && !globalEffectActive) {
      globalEffectActive = true;
    }
  }

  if(globalEffectActive) {
    globalEffectTime += dt;
    if(globalEffectTime > 3.f) {
      globalEffectActive = false;
      globalEffectTime = 0.f;
      globalEffectTimeoutActive = false;
      globalEffectTimeout = 1.f;
    }
  }
}

void Application::render() {
  glClear(GL_COLOR_BUFFER_BIT);

  pipeline.bind();

  glUniform1f(invImageAspectRatio_location, (float)webcam_height / webcam_width);
  glUniform1f(invScreenAspectRatio_location, (float)screen_height / screen_width);
  {
    const float aspect = (float)screen_width / screen_height;
    const float underscan = 1 - ((float)screen_height / screen_width) / ((float)webcam_height / webcam_width);
    const GLfloat mat[4*4] = {
      //column-major
      2.f/aspect, 0.f, 0.f, 0.f,
      0.f, 2.f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      underscan - 1.f, -1.f, 0.f, 1.f
    };
    glUniformMatrix4fv(viewProjectionMatrix_location, 1, GL_FALSE, mat);
  }
  {
    const float aspect = (float)screen_width / screen_height;
    const float underscan = 1 - ((float)screen_height / screen_width) / ((float)webcam_height / webcam_width);
    const GLfloat mat[4*4] = {
      //column-major
      .5f*aspect, 0.f, 0.f, 0.f,
      0.f, .5f, 0.f, 0.f,
      0.f, 0.f, 0.f, 0.f,
      (-.5f * (underscan - 1.f)) * aspect, .5f, 0.f, 1.f
    };
    glUniformMatrix4fv(invViewProjectionMatrix_location, 1, GL_FALSE, mat);
  }
  glUniform1f(globalTime_location, SDL_GetTicks() / 1000.f);
  glUniform1f(globalEffectTime_location, globalEffectTime);
  particleBuffer.draw();

  {
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) printf("opengl error: %d\n", error);
  }
}

void Application::webcamThreadFunc() {
  while(!kill_threads) {
    auto frame = webcam_buffer.startWrite();
    if(!webcam.getFrame(frame->data())) {
      std::cerr << "webcam lost, you need to restart the app\n";
      break;
    }
    webcam_buffer.finishWrite();
  }
}
