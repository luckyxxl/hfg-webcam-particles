#include "main.hpp"

#include "Application.hpp"
#include "Resources.hpp"
#include "graphics/Window.hpp"
#include "sound/Renderer.hpp"

int main(int argc, const char *argv[]) {
  Resources *resources = nullptr;
  graphics::Window *window = nullptr;
  sound::Renderer *soundRenderer = nullptr;
  Application *application = nullptr;

  resources = new Resources();
  if (!resources->create(argv[0])) {
    goto quit;
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not init SDL2",
                             SDL_GetError(), nullptr);
    goto quit;
  }

  window = new graphics::Window();
  if (!window->create()) {
    goto quit;
  }

  soundRenderer = new sound::Renderer();
  if (!soundRenderer->create()) {
    goto quit;
  }

  application = new Application();
  if (!application->create(resources, window, soundRenderer)) {
    goto quit;
  }

  {
    const auto size = window->getSize();
    application->reshape(std::get<0>(size), std::get<1>(size));
  }

  for (;;) {
    if (!application->handleEvents())
      break;

    application->update(1.f / 60.f);
    application->render();

    soundRenderer->update();

    window->swap();
  }

quit:
  if (application)
    application->destroy();
  delete application;
  if (soundRenderer)
    soundRenderer->destroy();
  delete soundRenderer;
  if (window)
    window->destroy();
  delete window;
  SDL_Quit();
  if (resources)
    resources->destroy();
  delete resources;

  return 0;
}
