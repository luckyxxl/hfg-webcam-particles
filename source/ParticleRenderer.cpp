#include "main.hpp"

#include "ParticleRenderer.hpp"
#include "Timeline.hpp"

void ParticleRenderer::reset() {
  graphicsPipeline.destroy();
  uniforms.clear();
}

void ParticleRenderer::setTimeline(std::unique_ptr<Timeline> _timeline) {
  reset();

  timeline = std::move(_timeline);

  state.clock.setPeriod(timeline->getPeriod());

  std::vector<UniformDescription> uniforms;
  ShaderBuilder vertexShader, fragmentShader;

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

  vertexShader.appendMainBody(R"glsl(
    vec3 initialPosition = vec3(texcoord, 0);
    initialPosition.x /= invImageAspectRatio;
    float pointSize = max(particleSize, 0.);

    vec3 position = initialPosition;
  )glsl");

  fragmentShader.appendMainBody(R"glsl(
    float v = pow(max(1. - 2. * length(gl_PointCoord - vec2(.5)), 0.), 1.5);
  )glsl");

  unsigned instanceId = 0;
  timeline->forEachInstance([&](const IEffect &i) {
    Uniforms instanceUniforms(uniforms, instanceId++);
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
#if 1
    vertexShader.appendMainBody("\n#line 0\n");
    fragmentShader.appendMainBody("\n#line 0\n");
#endif
    i.registerEffect(instanceUniforms, vertexShader, fragmentShader);
    vertexShader.appendMainBody("}");
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

  for (const auto &u : uniforms) {
    vertexShader.appendUniform(u);
    fragmentShader.appendUniform(u);
  }

  const auto vertexShaderSource = vertexShader.assemble();
  const auto fragmentShaderSource = fragmentShader.assemble();

  graphicsPipeline.create(vertexShaderSource.c_str(),
                          fragmentShaderSource.c_str());

  for (const auto &u : uniforms) {
    UniformElement newElement;
    newElement.location = graphicsPipeline.getUniformLocation(u.name.c_str());
    newElement.value = u.value;
    this->uniforms.push_back(newElement);
  }
}

void ParticleRenderer::update(float dt) {
  state.clock.frame(dt);
}

void ParticleRenderer::render(const RendererParameters &parameters) {
  RenderProps props(parameters, state);

  graphicsPipeline.bind();

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
    }
  }

  parameters.particle_buffer.draw();
}
