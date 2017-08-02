#pragma once

#include "Template.hpp"

struct RenderProps;

enum class GLSLType {
  Float,
  Vec2,
  Vec3,
  Vec4,
  Mat4,
};

struct UniformValue {
  GLSLType type;
  union Data {
    float f[16]; // can store mat4
  } data;

  UniformValue(float x) : type(GLSLType::Float) { data.f[0] = x; }
  UniformValue(float x, float y) : type(GLSLType::Vec2) { data.f[0] = x; data.f[1] = y; }
  UniformValue(float x, float y, float z) : type(GLSLType::Vec3) { data.f[0] = x; data.f[1] = y; data.f[2] = z; }
};

using UniformValueFunction = std::function<UniformValue(const RenderProps&)>;

struct UniformDescription {
  std::string name;
  GLSLType type;
  UniformValueFunction value;

  UniformDescription(const std::string &name, GLSLType type, UniformValueFunction value) : name(name), type(type), value(value) {}
};

//TODO: rename
class Uniforms {
  public:
  static constexpr auto undefinedId = std::numeric_limits<unsigned>::max();

  Uniforms(std::vector<UniformDescription> &uniforms, unsigned id = undefinedId) : uniforms(uniforms), id(id) {}

  const char *addUniform(const char *name, GLSLType type, UniformValueFunction value);

  private:
  std::vector<UniformDescription> &uniforms;
  unsigned id;
};

//simplification for specifying uniforms directly in the TEMPLATE(...).compile() list
#define UNIFORM(name, type, lambda) \
  { name, uniforms.addUniform(name, type, lambda) }

class ShaderBuilder {
  public:
  void appendUniform(const UniformDescription &uniform);
  void appendIn(const std::string &name, GLSLType type);
  void appendOut(const std::string &name, GLSLType type);
  void appendGlobal(const std::string &name, GLSLType type, const std::string &value);
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
