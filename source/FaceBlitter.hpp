#pragma once

#include "graphics/Framebuffer.hpp"
#include "graphics/ScreenRectBuffer.hpp"
#include "graphics/Pipeline.hpp"

class FaceBlitter {
public:
  void create(const graphics::ScreenRectBuffer *rectangle,
    uint32_t webcam_width, uint32_t webcam_height);
  void destroy();

  void blit(graphics::Texture &source, glm::vec2 sourceMin, glm::vec2 sourceMax,
    glm::vec2 targetMin, glm::vec2 targetMax, graphics::Texture &background);

  void update(float dt);
  void draw();

  void clear();

  graphics::Texture &getResultTexture() {
    return resultFramebuffer.getTexture();
  }

private:
  const graphics::ScreenRectBuffer *rectangle;

  struct BlitOperation {
    glm::vec2 bufferCenter;
    glm::vec2 targetCenter;
    glm::vec2 extend; // same for buffer and target
    float time;
  };

  std::vector<BlitOperation> blitOperations;

  glm::vec2 nextBufferOrigin;
  float currentBufferRowYSize;

  graphics::Pipeline blitToBufferPipeline;
  GLint blitToBufferPipeline_sourceCenter_location;
  GLint blitToBufferPipeline_sourceExtend_location;
  GLint blitToBufferPipeline_targetCenter_location;
  GLint blitToBufferPipeline_targetExtend_location;
  GLint blitToBufferPipeline_source_location;
  GLint blitToBufferPipeline_background_location;

  graphics::Pipeline blitToResultPipeline;
  GLint blitToResultPipeline_sourceCenter_location;
  GLint blitToResultPipeline_sourceExtend_location;
  GLint blitToResultPipeline_targetCenter_location;
  GLint blitToResultPipeline_targetExtend_location;
  GLint blitToResultPipeline_source_location;
  GLint blitToResultPipeline_visibility_location;

  graphics::Framebuffer facesBuffer;
  graphics::Framebuffer overlayFramebuffer;
  graphics::Framebuffer resultFramebuffer;
};
