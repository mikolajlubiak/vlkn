#include "vlkn_window.hpp"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace vlkn {

VlknWindow::VlknWindow(const uint32_t w, const uint32_t h,
                       const std::string name)
    : width(w), height(h), windowName(name) {
  initWindow();
}

VlknWindow::~VlknWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void VlknWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

} // namespace vlkn
