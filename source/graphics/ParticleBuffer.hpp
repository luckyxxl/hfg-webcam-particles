#pragma once

namespace graphics {

struct Particle {
  float position[2];
  float rgb[3];
  float hsv[3];
  float localEffectStrength;
};

class ParticleBuffer {
  public:
  bool create(size_t particleCount);
  void destroy();

  size_t getParticleCount() const { return particleCount; }

  void setParticleData(const Particle *particleData, size_t particleDataSize);

  void draw() const;

  private:
  size_t particleCount = 0u;

  GLuint vertexBuffer = 0;
  GLuint vertexArray = 0;
};

}
