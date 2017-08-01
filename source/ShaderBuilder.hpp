#pragma once

struct EffectInstance;

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

using UniformValueFunction = std::function<UniformValue(const EffectInstance&)>;

class ShaderBuilder {
  public:
  class UniformMap {
    public:
    UniformMap(unsigned instanceId);

    void addUniform(const char *name, GLSLType type, UniformValueFunction value);

    private:
    unsigned instanceId;

    struct Element {
      const char *originalName;
      std::string mappedName;
      GLSLType type;
      UniformValueFunction value;
    };
    std::vector<Element> elements;

    friend class ShaderBuilder;
  };

  void appendMainBody(const UniformMap &uniformMap, const char *source);

  std::string assemble() const;

  private:
  struct UniformElement {
    std::string name;
    GLSLType type;
    UniformValueFunction value;
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
