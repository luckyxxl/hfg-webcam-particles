#include "main.hpp"

#include "Resources.hpp"
#include "Webcam.hpp"
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
  SDL_Window *window = nullptr;
  SDL_GLContext gl_context = nullptr;
  Webcam *webcam = nullptr;
  Application *application = nullptr;

  resources = new Resources(argv[0]);

  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not init SDL2", SDL_GetError(), nullptr);
    goto quit;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  window = SDL_CreateWindow("hfg-webcam-particles", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if(!window) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window", SDL_GetError(), nullptr);
    goto quit;
  }

  gl_context = SDL_GL_CreateContext(window);
  if(!gl_context) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create OpenGL context", SDL_GetError(), nullptr);
    goto quit;
  }

  SDL_GL_SetSwapInterval(1);

  webcam = new Webcam();

  application = new Application(resources, webcam);

  {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    application->reshape(w, h);
  }

  for(;;) {
    if(!handleEvents(application)) break;

    application->update(1.f / 60.f);
    application->render();

    SDL_GL_SwapWindow(window);
  }

  quit:
  delete application;
  delete webcam;
  if(gl_context) SDL_GL_DeleteContext(gl_context);
  if(window) SDL_DestroyWindow(window);
  SDL_Quit();
  delete resources;

  return 0;
}
