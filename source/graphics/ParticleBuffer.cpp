#include "main.hpp"

#include "ParticleBuffer.hpp"

namespace graphics {

bool ParticleBuffer::create(size_t particleCount) {
  this->particleCount = particleCount;

  glGenVertexArrays(1, &vertexArray);
  glBindVertexArray(vertexArray);

  glGenBuffers(1, &vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, particleCount * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, rgb)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, hsv)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<void*>(offsetof(Particle, localEffectStrength)));
  glEnableVertexAttribArray(3);

  return true;
}

void ParticleBuffer::destroy() {
  glDeleteVertexArrays(1, &vertexArray);
  glDeleteBuffers(1, &vertexBuffer);
  particleCount = 0u;
}

void ParticleBuffer::setParticleData(const Particle *particleData, size_t particleDataSize) {
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, particleDataSize * sizeof(Particle), particleData);
}

void ParticleBuffer::draw() const {
  glBindVertexArray(vertexArray);
  glDrawArrays(GL_POINTS, 0, particleCount);
}

}
