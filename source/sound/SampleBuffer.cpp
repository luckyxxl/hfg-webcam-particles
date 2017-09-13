#include "main.hpp"

#include "Resources.hpp"
#include "SampleBuffer.hpp"

namespace sound {

bool SampleBuffer::loadFromFile(Resources *resources, const char *filename) {
  auto data = resources->readWholeBinaryFile(filename);

  // http://soundfile.sapp.org/doc/WaveFormat/
  struct Header {
    uint32_t chunkID;
    uint32_t chunkSize;
    uint32_t format;

    uint32_t subchunk1ID;
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    uint32_t subchunk2ID;
    uint32_t subchunk2Size;
  };

  auto header = reinterpret_cast<const Header *>(data.data());

  if (header->chunkID != 0x46464952 || header->format != 0x45564157 ||
      header->subchunk1ID != 0x20746d66 || header->subchunk2ID != 0x61746164) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open sample",
                             "Unsupported file", NULL);
    return false;
  }

  if (header->audioFormat != 1) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open sample",
                             "Unsupported audio format", NULL);
    return false;
  }

  if (header->numChannels != 1 && header->numChannels != 2) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open sample",
                             "Unsupported channel count", NULL);
    return false;
  }

  if (header->sampleRate != 48000) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open sample",
                             "Unsupported sample rate", NULL);
    return false;
  }

  if (header->bitsPerSample != 16 && header->bitsPerSample != 24) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not open sample",
                             "Unsupported bit depth", NULL);
    return false;
  }

  channels = header->numChannels;

  const auto elementsCount = header->subchunk2Size / (header->bitsPerSample / 8);
  const auto padElements = elementsCount == 0u ? 4u : elementsCount % 4 != 0 ? 4 - elementsCount % 4 : 0;

  buffer.resize(elementsCount + padElements);
  assert(reinterpret_cast<size_t>(buffer.data()) % (4u * sizeof(float)) == 0u); // sse alignment requirement

  switch (header->bitsPerSample) {
  case 16: {
    auto samples =
        reinterpret_cast<const int16_t *>(data.data() + sizeof(Header));
    for (auto i = 0u; i < elementsCount; ++i) {
      buffer[i] = samples[i] / float(1 << (16 - 1));
    }
  } break;

  case 24: {
    struct sample24_t {
      uint8_t b[3];
    };
    static_assert(sizeof(sample24_t) == 3);
    auto samples =
        reinterpret_cast<const sample24_t *>(data.data() + sizeof(Header));
    for (auto i = 0u; i < elementsCount; ++i) {
      const auto s = samples[i];
      auto v = (s.b[2] & 0x7F) << 16 | s.b[1] << 8 | s.b[0];
      if (s.b[2] & 0x80)
        v -= 1 << (24 - 1);
      buffer[i] = v / float(1 << (24 - 1));
    }
  } break;
  }

  for(auto i = 0u; i < padElements; ++i) {
    buffer[elementsCount + i] = 0.f;
  }

  return true;
}

} // namespace sound
