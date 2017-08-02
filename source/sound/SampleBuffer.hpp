#pragma once

class Resources;

namespace sound {

class SampleBuffer {
public:
  bool loadFromFile(Resources *resources, const char *filename);

  const float *getBuffer() const { return buffer.data(); }

  uint32_t getBufferLengthSamples() const {
    return buffer.size() / getChannels();
  }

  uint8_t getChannels() const { return channels; }

private:
  std::vector<float> buffer;
  uint8_t channels;
};

} // namespace sound
