#include "main.hpp"

#include "ParticleRenderer.hpp"
#include "Timeline.hpp"

ParticleRenderer::ParticleRenderer(std::default_random_engine &random)
    : props(state, random) {}

void ParticleRenderer::reset() {
  graphicsPipeline.destroy();
  uniforms.clear();
}

void ParticleRenderer::setTimeline(std::unique_ptr<Timeline> _timeline) {
  reset();

  timeline = std::move(_timeline);

  std::vector<UniformDescription> uniforms;
  ShaderBuilder vertexShader, fragmentShader;

  uniforms.emplace_back("invImageAspectRatio", GLSLType::Float,
                        [](const RenderProps &props) {
                          // TODO
                          return UniformValue(1.f);
                        });
  uniforms.emplace_back("invScreenAspectRatio", GLSLType::Float,
                        [](const RenderProps &props) {
                          // TODO
                          return UniformValue(1.f);
                        });
  uniforms.emplace_back("viewProjectionMatrix", GLSLType::Mat4,
                        [](const RenderProps &props) {
                          // TODO
                          return UniformValue(1.f);
                        });
  uniforms.emplace_back("invViewProjectionMatrix", GLSLType::Mat4,
                        [](const RenderProps &props) {
                          // TODO
                          return UniformValue(1.f);
                        });
  uniforms.emplace_back("particleSize", GLSLType::Float,
                        [](const RenderProps &props) {
                          // TODO
                          return UniformValue(1.f);
                        });
  uniforms.emplace_back("globalTime", GLSLType::Float,
                        [](const RenderProps &props) {
                          return UniformValue(props.state.clock.getTime());
                        });

  vertexShader.appendIn("texcoord", GLSLType::Vec2);
  vertexShader.appendIn("rgb", GLSLType::Vec3);
  vertexShader.appendIn("hsv", GLSLType::Vec3);

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
    initialPosition.y *= invImageAspectRatio;
    float pointSize = max(particleSize, 0.);

    vec3 position = initialPosition;
  )glsl");

  fragmentShader.appendMainBody(R"glsl(
    float v = pow(max(1. - 2. * length(gl_PointCoord - vec2(.5)), 0.), 1.5);
  )glsl");

  unsigned instanceId = 0;
  timeline->forEachInstance([&](const IEffect &i) {
    Uniforms instanceUniforms(uniforms, instanceId++);
    vertexShader.appendMainBody(("if(" + std::to_string(i.getTimeBegin()) +
                                 "<= globalTime && globalTime <=" +
                                 std::to_string(i.getTimeEnd()) + ") {")
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

  std::cout << vertexShaderSource << "\n" << fragmentShaderSource << "\n";

  graphicsPipeline.create(vertexShaderSource.c_str(),
                          fragmentShaderSource.c_str());

  for (const auto &u : uniforms) {
    UniformElement newElement;
    newElement.location = graphicsPipeline.getUniformLocation(u.name.c_str());
    newElement.value = u.value;
    this->uniforms.push_back(newElement);
  }
}

void ParticleRenderer::update(float dt) {}

void ParticleRenderer::render() {
  graphicsPipeline.bind();

  for (const auto &uniform : uniforms) {
    const auto value = uniform.value(props);
    switch (value.type) {
    case GLSLType::Float:
      glUniform1fv(uniform.location, 1, value.data.f);
      break;
    case GLSLType::Vec2:
      glUniform2fv(uniform.location, 1, value.data.f);
      break;
    case GLSLType::Vec3:
      glUniform3fv(uniform.location, 1, value.data.f);
      break;
    case GLSLType::Vec4:
      glUniform4fv(uniform.location, 1, value.data.f);
      break;
    case GLSLType::Mat4:
      glUniformMatrix4fv(uniform.location, 1, GL_FALSE, value.data.f);
      break;
    }
  }
}
