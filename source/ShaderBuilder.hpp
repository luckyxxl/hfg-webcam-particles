#pragma once

#include "Template.hpp"

struct RenderProps;

enum class GLSLType {
  Float,
  Vec2,
  Vec3,
  Vec4,
  Mat4,
  Sampler2D,
};

struct UniformValue {
  GLSLType type;
  union Data {
    float f;
    glm::vec2 v2;
    glm::vec3 v3;
    glm::vec4 v4;
    glm::mat4 m4;

    Data() : m4(0.f) {}
  } data;

  UniformValue(float x) : type(GLSLType::Float) { data.f = x; }
  UniformValue(glm::vec2 x) : type(GLSLType::Vec2) { data.v2 = x; }
  UniformValue(glm::vec3 x) : type(GLSLType::Vec3) { data.v3 = x; }
  UniformValue(glm::mat4 x) : type(GLSLType::Mat4) { data.m4 = x; }

  static UniformValue Sampler2D() {
    UniformValue result(0.f);
    result.type = GLSLType::Sampler2D;
    return result;
  }
};

using UniformValueFunction = std::function<UniformValue(const RenderProps &)>;

struct UniformDescription {
  std::string name;
  GLSLType type;
  UniformValueFunction value;

  UniformDescription(const std::string &name, GLSLType type,
                     UniformValueFunction value)
      : name(name), type(type), value(value) {}
};

// TODO: rename
class Uniforms {
public:
  static constexpr auto undefinedId = std::numeric_limits<unsigned>::max();

  Uniforms(std::vector<UniformDescription> &uniforms, unsigned id = undefinedId)
      : uniforms(uniforms), id(id) {}

  std::string addUniform(const char *name, GLSLType type,
                         UniformValueFunction value);

private:
  std::vector<UniformDescription> &uniforms;
  unsigned id;
};

// simplification for specifying uniforms directly in the
// TEMPLATE(...).compile() list
#define UNIFORM(name, type, lambda)                                            \
  { name, uniforms.addUniform(name, type, lambda) }

class ShaderBuilder {
public:
  void appendUniform(const UniformDescription &uniform);
  void appendIn(const std::string &name, GLSLType type);
  void appendOut(const std::string &name, GLSLType type);
  void appendGlobal(const std::string &name, GLSLType type,
                    const std::string &value);
  void appendFunction(const std::string &function);
  void appendMainBody(const char *source);

  std::string assemble() const;

private:
  struct UniformElement {
    std::string name;
    GLSLType type;
  };
  std::vector<UniformElement> uniforms;

  struct InOutElement {
    std::string name;
    GLSLType type;
  };
  std::vector<InOutElement> ins;
  std::vector<InOutElement> outs;

  struct GlobalElement {
    std::string name;
    GLSLType type;
    std::string value;
  };
  std::vector<GlobalElement> globals;

  std::vector<std::string> functions;

  std::string mainBody;
};
