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

ShaderBuilder::UniformMap::UniformMap(unsigned instanceId) : instanceId(instanceId) {
}

void ShaderBuilder::UniformMap::addUniform(const char *name, GLSLType type, UniformValueFunction value) {
  Element newElement;
  newElement.originalName = name;
  newElement.mappedName = std::string(name) + "_" + std::to_string(instanceId);
  newElement.type = type;
  newElement.value = value;
  elements.push_back(newElement);
}

void ShaderBuilder::appendMainBody(const UniformMap &uniformMap, const char *source) {
  std::string result = source;
  for(auto &u : uniformMap.elements) {
    const auto toReplace = "${" + std::string(u.originalName) + "}";
    const auto replaceWith = u.mappedName;

    std::string::size_type p = 0;
    while((p = result.find(toReplace, p)) != std::string::npos) {
      result.replace(p, toReplace.length(), replaceWith);
      p += replaceWith.length();
    }
  }

  mainBody += result;

  for(auto &u : uniformMap.elements) {
    UniformElement uniform;
    uniform.name = u.mappedName;
    uniform.type = u.type;
    uniform.value = u.value;
    uniforms.push_back(uniform);
  }
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
