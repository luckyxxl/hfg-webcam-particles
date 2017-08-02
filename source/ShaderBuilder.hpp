#pragma once

#include "Template.hpp"

struct RenderProps;

enum class GLSLType {
  Float,
  Vec2,
  Vec3,
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

//TODO: rename
class Uniforms {
  public:
  static constexpr auto undefinedId = std::numeric_limits<unsigned>::max();
  Uniforms(unsigned id = undefinedId) : id(id) {}

  const char *addUniform(const char *name, GLSLType type, UniformValueFunction value);

  struct Element {
    std::string name;
    GLSLType type;
    UniformValueFunction value;
  };

  const std::vector<Element> &getUniforms() const { return uniforms; }

  private:
  unsigned id;
  std::vector<Element> uniforms;
};

class ShaderBuilder {
  public:
  void appendUniforms(const Uniforms &uniforms);
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
