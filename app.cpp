#include "app.hpp"

#include <GLFW/glfw3.h>

namespace vlkn {

void App::run() {
  while (!vlknWindow.shouldClose()) {
    glfwPollEvents();
  }
}

} // namespace vlkn
