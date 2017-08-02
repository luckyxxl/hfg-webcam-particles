#include "main.hpp"

#include "ShaderBuilder.hpp"

static const char *glslTypeString(GLSLType type) {
  switch(type) {
    case GLSLType::Float: return "float";
    case GLSLType::Vec2: return "vec2";
    case GLSLType::Vec3: return "vec3";
    case GLSLType::Mat4: return "mat4";
  }
  return "ERROR";
}

const char *Uniforms::addUniform(const char *name, GLSLType type, UniformValueFunction value) {
  Element newElement;
  newElement.name = std::string(name) + (id != undefinedId ? "_" + std::to_string(id) : "");
  newElement.type = type;
  newElement.value = value;
  uniforms.push_back(newElement);
  return uniforms.back().name.c_str();
}

void ShaderBuilder::appendUniforms(const Uniforms &_uniforms) {
  for(auto &u : _uniforms.getUniforms()) {
    UniformElement newElement;
    newElement.name = u.name;
    newElement.type = u.type;
    uniforms.push_back(newElement);
  }
}

void ShaderBuilder::appendMainBody(const char *source) {
  mainBody += source;
}

std::string ShaderBuilder::assemble() const {
  std::ostringstream result;

  result << "#version 330 core\n";
  result << "\n";

  for(auto &u : uniforms) {
    result << "uniform " << glslTypeString(u.type) << " " << u.name << ";\n";
  }

  result << "\n";

  for(auto &i : ins) {
    result << "in " << glslTypeString(i.type) << " " << i.name << ";\n";
  }

  result << "\n";

  for(auto &o : outs) {
    result << "out " << glslTypeString(o.type) << " " << o.name << ";\n";
  }

  result << "\n";

  for(auto &o : outs) {
    result << "out " << glslTypeString(o.type) << " " << o.name << ";\n";
  }

  result << "\n";

  for(auto &g : globals) {
    result << "const " << glslTypeString(g.type) << " " << g.name << " = " << g.value << ";\n";
  }

  result << "\n";

  for(auto &f : functions) {
    result << f << "\n";
  }

  result << "\n";

  result << "void main() {\n";
  result << mainBody;
  result << "}\n";

  return result.str();
}
