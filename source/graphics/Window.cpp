#include "main.hpp"

#include "Window.hpp"

namespace graphics {

static const char *windowTitle = "webcam-particles";

#ifdef OPENGL_DEBUG
static void APIENTRY openglDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
  std::cout << message << "\n";
}
#endif

bool Window::create(uint32_t width, uint32_t height, bool fullscreen) {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef OPENGL_DEBUG
  std::cout << "OPENGL_DEBUG is enabled!\n";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

  auto flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  if(fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, width, height, flags);
  if (!window) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Could not create window",
                             SDL_GetError(), nullptr);
    return false;
  }

  gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                             "Could not create OpenGL context", SDL_GetError(),
                             nullptr);
    return false;
  }

  std::cout << "OpenGL:\n"
      << "\tVersion: " << glGetString(GL_VERSION) << "\n"
      << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n"
      << "\tRenderer: " << glGetString(GL_RENDERER) << "\n"
      << "\tVendor: " << glGetString(GL_VENDOR) << "\n";

#ifdef OPENGL_DEBUG
  glDebugMessageCallback(openglDebugMessageCallback, nullptr);
#endif

  SDL_GL_SetSwapInterval(1);

  if(fullscreen) SDL_ShowCursor(SDL_DISABLE);

  return true;
}

void Window::destroy() {
  if (gl_context)
    SDL_GL_DeleteContext(gl_context);
  if (window)
    SDL_DestroyWindow(window);
}

std::tuple<uint32_t, uint32_t> Window::getSize() const {
  int w, h;
  SDL_GetWindowSize(window, &w, &h);
  return std::make_tuple(w, h);
}

void Window::swap() {
  SDL_GL_SwapWindow(window);

  updateFPS();
}

void Window::updateFPS() {
  const auto lastTime = fps.lastTime;
  const auto currentTime = SDL_GetTicks();

  fps.counter += 1;

  if (currentTime - lastTime >= 1000) {
    char title[64];
    strcpy(title, windowTitle);
    sprintf(title + strlen(windowTitle), " (%d FPS)", fps.counter);
    SDL_SetWindowTitle(window, title);
    fps.lastTime = currentTime;
    fps.counter = 0;
  }
}

bool Window::toggleFullscreen() {
  auto flag = SDL_WINDOW_FULLSCREEN_DESKTOP; // FIXME this can be
                                             // SDL_WINDOW_FULLSCREEN, too
  bool isFullscreen = SDL_GetWindowFlags(window) & flag;
  bool success = !SDL_SetWindowFullscreen(window, isFullscreen ? 0 : flag);
  SDL_ShowCursor(isFullscreen ? SDL_ENABLE : SDL_DISABLE);
  return success;
}

} // namespace graphics
