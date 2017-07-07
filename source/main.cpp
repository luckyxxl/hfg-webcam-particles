#include "main.hpp"

#include "Resources.hpp"
#include "graphics/Window.hpp"
#include "Application.hpp"

static bool handleEvents(Application *application) {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT:
      return false;
      break;

      case SDL_WINDOWEVENT:
      switch(event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        application->reshape(event.window.data1, event.window.data2);
        break;
      }
      break;

      default:
      application->handleEvent(event);
      break;
    }
  }

  return true;
}

int main(int argc, const char *argv[]) {
  Resources *resources = nullptr;
  graphics::Window *window = nullptr;
  Application *application = nullptr;

  resources = new Resources();
  if(!resources->create(argv[0])) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not initialize resources", "Could not initialize resources", NULL);
    goto quit;
  }

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not init SDL2", SDL_GetError(), nullptr);
    goto quit;
  }

  window = new graphics::Window();
  if(!window->create()) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", "Could not create window", NULL);
    goto quit;
  }

  application = new Application();
  if(!application->create(resources)) {
    goto quit;
  }

  {
    const auto size = window->getSize();
    application->reshape(std::get<0>(size), std::get<1>(size));
  }

  for(;;) {
    if(!handleEvents(application)) break;

    application->update(1.f / 60.f);
    application->render();

    window->swap();
  }

  quit:
  if(application) application->destroy();
  delete application;
  if(window) window->destroy();
  delete window;
  SDL_Quit();
  if(resources) resources->destroy();
  delete resources;

  return 0;
}
