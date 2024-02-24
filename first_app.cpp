#include "first_app.hpp"

#include <GLFW/glfw3.h>

namespace vlkn {

void FirstApp::run() {
  while (!vlknWindow.shouldClose()) {
    glfwPollEvents();
  }
}

} // namespace vlkn
