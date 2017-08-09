#include "main.hpp"

#include "ParticleRenderer.hpp"
#include "Timeline.hpp"
#include "effects/AccumulationEffect.hpp"

void ParticleRenderer::GlobalState::create() {
  screenRectBuffer.create();

  particleFramebuffer.create(1, 1);
  accumulationFramebuffer.create(1, 1);
  resultFramebuffer.create(1, 1);

  resultGraphicsPipeline.create(R"glsl(
    #version 330 core
    layout(location=0) in vec2 position;
    void main() {
      gl_Position = vec4(position, 0, 1);
    }
  )glsl", R"glsl(
    #version 330 core
    uniform sampler2D resultTexture;
    out vec4 frag_color;
    void main() {
      vec3 color = texelFetch(resultTexture, ivec2(gl_FragCoord.xy), 0).rgb;
      frag_color = vec4(color, 1);
    }
  )glsl", false);
  resultGraphicsPipeline_resultTexture_location =
      resultGraphicsPipeline.getUniformLocation("resultTexture");
}

void ParticleRenderer::GlobalState::destroy() {
  resultGraphicsPipeline.destroy();

  resultFramebuffer.destroy();
  accumulationFramebuffer.destroy();
  particleFramebuffer.destroy();

  screenRectBuffer.destroy();
}

void ParticleRenderer::GlobalState::reshape(uint32_t width, uint32_t height) {
  particleFramebuffer.resize(width, height);
  accumulationFramebuffer.resize(width, height);
  resultFramebuffer.resize(width, height);
}


void ParticleRenderer::reset() {
  accumulationActive = false;
  graphicsPipeline.destroy();
  accGraphicsPipeline.destroy();
  uniforms.clear();
  accUniforms.clear();
}

void ParticleRenderer::setTimeline(std::unique_ptr<Timeline> _timeline) {
  reset();

  timeline = std::move(_timeline);

  state.clock.setPeriod(timeline->getPeriod());

  std::vector<UniformDescription> uniforms, accUniforms;
  ShaderBuilder vertexShader, fragmentShader;
  ShaderBuilder accShader;

  uniforms.emplace_back("invImageAspectRatio", GLSLType::Float,
                        [](const RenderProps &props) {
                          return UniformValue((float)props.webcam_height / props.webcam_width);
                        });
  uniforms.emplace_back("invScreenAspectRatio", GLSLType::Float,
                        [](const RenderProps &props) {
                          return UniformValue((float)props.screen_height / props.screen_width);
                        });
  uniforms.emplace_back("viewProjectionMatrix", GLSLType::Mat4,
                        [](const RenderProps &props) {
                          const auto aspect = (float)props.screen_width / props.screen_height;
                          const auto underscan = 1 - ((float)props.screen_height / props.screen_width) /
                                                         ((float)props.webcam_height / props.webcam_width);
                          // clang-format off
                          return UniformValue(glm::mat4(
                              2.f / aspect, 0.f, 0.f, 0.f,
                              0.f, 2.f, 0.f, 0.f,
                              0.f, 0.f, 0.f, 0.f,
                              underscan - 1.f, -1.f, 0.f, 1.f
                          ));
                          // clang-format on
                        });
  uniforms.emplace_back("invViewProjectionMatrix", GLSLType::Mat4,
                        [](const RenderProps &props) {
                          const auto aspect = (float)props.screen_width / props.screen_height;
                          const auto underscan = 1 - ((float)props.screen_height / props.screen_width) /
                                                         ((float)props.webcam_height / props.webcam_width);
                          // clang-format off
                          return UniformValue(glm::mat4(
                            .5f * aspect, 0.f, 0.f, 0.f,
                            0.f, .5f, 0.f, 0.f,
                            0.f, 0.f, 0.f, 0.f,
                            (-.5f * (underscan - 1.f)) * aspect, .5f, 0.f, 1.f
                          ));
                          // clang-format on
                        });
  uniforms.emplace_back("particleSize", GLSLType::Float,
                        [](const RenderProps &props) {
                          //TODO: particleScaling config
                          return UniformValue(((float)props.screen_height / props.webcam_height) * 2 /* * particleScaling*/);
                        });
  uniforms.emplace_back("globalTime", GLSLType::Float,
                        [](const RenderProps &props) {
                          return UniformValue(props.state.clock.getTime());
                        });

  accUniforms.emplace_back("particleTexture", GLSLType::Sampler2D,
                           [](const RenderProps &props) {
                             return UniformValue::Sampler2D();
                           });
  accUniforms.emplace_back("historyTexture", GLSLType::Sampler2D,
                           [](const RenderProps &props) {
                             return UniformValue::Sampler2D();
                           });
  accUniforms.emplace_back("globalTime", GLSLType::Float,
                           [](const RenderProps &props) {
                             return UniformValue(props.state.clock.getTime());
                           });

  // keep in sync with graphics::Pipeline
  vertexShader.appendIn("texcoord", GLSLType::Vec2);
  vertexShader.appendIn("rgb", GLSLType::Vec3);
  vertexShader.appendIn("hsv", GLSLType::Vec3);
  vertexShader.appendIn("foregroundMask", GLSLType::Float);

  vertexShader.appendOut("color", GLSLType::Vec3);

  vertexShader.appendGlobal("PI", GLSLType::Float, std::to_string(PI));

  vertexShader.appendFunction(R"glsl(
    vec2 getDirectionVector(float angle) {
      return vec2(cos(angle), sin(angle));
    }
  )glsl");

  fragmentShader.appendIn("color", GLSLType::Vec3);

  fragmentShader.appendOut("frag_color", GLSLType::Vec4);

  accShader.appendIn("texcoord", GLSLType::Vec2);

  accShader.appendOut("frag_color", GLSLType::Vec4);

  vertexShader.appendMainBody(R"glsl(
    vec3 initialPosition = vec3(texcoord, 0);
    initialPosition.x /= invImageAspectRatio;
    float pointSize = max(particleSize, 0.);

    vec3 position = initialPosition;
  )glsl");

  fragmentShader.appendMainBody(R"glsl(
    float v = pow(max(1. - 2. * length(gl_PointCoord - vec2(.5)), 0.), 1.5);
  )glsl");

  accShader.appendMainBody(R"glsl(
    vec3 particleColor = texelFetch(particleTexture, ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 historyColor = texelFetch(historyTexture, ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 accumulationResult = vec3(0.);
    int activeAgents = 0;
  )glsl");

  unsigned instanceId = 0;
  timeline->forEachInstance([&](const IEffect &i) {
    Uniforms instanceUniforms(uniforms, instanceId);
    Uniforms accInstanceUniforms(accUniforms, instanceId);

    EffectRegistrationData registrationData(instanceUniforms, vertexShader,
                                            fragmentShader, accInstanceUniforms,
                                            accShader);

    const auto timeBegin = instanceUniforms.addUniform("timeBegin",
      GLSLType::Float, [&i](const RenderProps &props) {
        return UniformValue(i.timeBegin);
      });
    const auto timeEnd = instanceUniforms.addUniform("timeEnd",
      GLSLType::Float, [&i](const RenderProps &props) {
        return UniformValue(i.timeEnd);
      });

    vertexShader.appendMainBody(TEMPLATE("if(${timeBegin} <= globalTime && "
                                         "globalTime <= ${timeEnd}) {")
                                         .compile({
                                           {"timeBegin", timeBegin},
                                           {"timeEnd", timeEnd},
                                         })
                                    .c_str());

    if(i.isAccumulationEffect()) {
      accumulationActive = true;

      const auto accTimeBegin = accInstanceUniforms.addUniform("timeBegin",
        GLSLType::Float, [&i](const RenderProps &props) {
          return UniformValue(i.timeBegin);
        });
      const auto accTimeEnd = accInstanceUniforms.addUniform("timeEnd",
        GLSLType::Float, [&i](const RenderProps &props) {
          return UniformValue(i.timeEnd);
        });

      accShader.appendMainBody(TEMPLATE(R"glsl(
        if(${timeBegin} <= globalTime && globalTime <= ${timeEnd}) {
          activeAgents++;
          vec3 accumulationEffectResult;
        )glsl").compile({
                         {"timeBegin", accTimeBegin},
                         {"timeEnd", accTimeEnd},
                        }).c_str());
    }

    vertexShader.appendMainBody("\n#line 0\n");
    fragmentShader.appendMainBody("\n#line 0\n");
    accShader.appendMainBody("\n#line 0\n");

    i.registerEffect(registrationData);

    vertexShader.appendMainBody("}");

    if(i.isAccumulationEffect()) {
      const auto accFadeWeight = accInstanceUniforms.addUniform("fadeWeight",
        GLSLType::Float, [&i](const RenderProps &props) {
          const auto t = props.state.clock.getTime();
          const auto timeBegin = i.timeBegin;
          const auto timeEnd = i.timeEnd;
          const auto fadeIn = static_cast<const IAccumulationEffect&>(i).fadeIn;
          const auto fadeOut = static_cast<const IAccumulationEffect&>(i).fadeOut;
          return UniformValue(
              t < (timeBegin + fadeIn) ? (t - timeBegin) / fadeIn :
              t > (timeEnd - fadeOut) ? 1 - (t - (timeEnd - fadeOut)) / fadeOut :
              1);
        });

      accShader.appendMainBody(TEMPLATE(R"glsl(
          accumulationResult += mix(particleColor, accumulationEffectResult, ${fadeWeight});
        }
        )glsl").compile({
                         {"fadeWeight", accFadeWeight}
                        }).c_str());
    }

    instanceId++;
  });

  vertexShader.appendMainBody(R"glsl(
    color = rgb;
    gl_PointSize = pointSize;
    gl_Position = viewProjectionMatrix * vec4(position, 1.);
  )glsl");

  // TODO: different overlap modes
  fragmentShader.appendMainBody(R"glsl(
    frag_color = vec4(color * v, 1);
  )glsl");

  accShader.appendMainBody(R"glsl(
    if (activeAgents > 0) {
      accumulationResult /= float(activeAgents);
    } else {
      accumulationResult = particleColor;
    }

    frag_color = vec4(accumulationResult, 1);
  )glsl");

  for (const auto &u : uniforms) {
    vertexShader.appendUniform(u);
    fragmentShader.appendUniform(u);
  }

  for (const auto &u : accUniforms) {
    accShader.appendUniform(u);
  }

  const auto vertexShaderSource = vertexShader.assemble();
  const auto fragmentShaderSource = fragmentShader.assemble();
  const auto accShaderSource = accShader.assemble();

  graphicsPipeline.create(vertexShaderSource.c_str(),
                          fragmentShaderSource.c_str(), true);

  accGraphicsPipeline.create(R"glsl(
    #version 330 core
    layout(location=0) in vec2 position;
    void main() {
      gl_Position = vec4(position, 0, 1);
    }
  )glsl", accShaderSource.c_str(), false);
  accGraphicsPipeline_particleTexture_location =
      accGraphicsPipeline.getUniformLocation("particleTexture");
  accGraphicsPipeline_historyTexture_location =
      accGraphicsPipeline.getUniformLocation("historyTexture");

  for (const auto &u : uniforms) {
    UniformElement newElement;
    newElement.location = graphicsPipeline.getUniformLocation(u.name.c_str());
    newElement.value = u.value;
    this->uniforms.push_back(newElement);
  }

  for (const auto &u : accUniforms) {
    UniformElement newElement;
    newElement.location = accGraphicsPipeline.getUniformLocation(u.name.c_str());
    newElement.value = u.value;
    this->accUniforms.push_back(newElement);
  }
}

void ParticleRenderer::update(float dt) {
  state.clock.frame(dt);
}

void ParticleRenderer::render(GlobalState &globalState, const RendererParameters &parameters) {
  RenderProps props(parameters, state);

  if(accumulationActive) {
    globalState.particleFramebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    graphicsPipeline.bind();
    loadUniforms(uniforms, props);
    parameters.particle_buffer.draw();

    std::swap(globalState.accumulationFramebuffer, globalState.resultFramebuffer);

    globalState.resultFramebuffer.bind();
    accGraphicsPipeline.bind();
    globalState.particleFramebuffer.getTexture().bind(0);
    glUniform1i(accGraphicsPipeline_particleTexture_location, 0);
    globalState.accumulationFramebuffer.getTexture().bind(1);
    glUniform1i(accGraphicsPipeline_historyTexture_location, 1);
    loadUniforms(accUniforms, props);
    globalState.screenRectBuffer.draw();

    graphics::Framebuffer::unbind();
    globalState.resultGraphicsPipeline.bind();
    globalState.resultFramebuffer.getTexture().bind(0);
    glUniform1i(globalState.resultGraphicsPipeline_resultTexture_location, 0);
    globalState.screenRectBuffer.draw();

    graphics::Texture::unbind(0);
    graphics::Texture::unbind(1);
  } else {
    graphicsPipeline.bind();
    loadUniforms(uniforms, props);
    parameters.particle_buffer.draw();
  }
}

void ParticleRenderer::loadUniforms(const std::vector<UniformElement> &uniforms, const RenderProps &props) {
  for (const auto &uniform : uniforms) {
    const auto value = uniform.value(props);
    switch (value.type) {
    case GLSLType::Float:
      glUniform1fv(uniform.location, 1, &value.data.f);
      break;
    case GLSLType::Vec2:
      glUniform2fv(uniform.location, 1, &value.data.v2[0]);
      break;
    case GLSLType::Vec3:
      glUniform3fv(uniform.location, 1, &value.data.v3[0]);
      break;
    case GLSLType::Vec4:
      glUniform4fv(uniform.location, 1, &value.data.v4[0]);
      break;
    case GLSLType::Mat4:
      glUniformMatrix4fv(uniform.location, 1, GL_FALSE, &value.data.m4[0][0]);
      break;
    case GLSLType::Sampler2D:
      break;
    }
  }
}
