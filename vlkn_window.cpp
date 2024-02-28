#include "vlkn_window.hpp"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>

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

void VlknWindow::createWindowSurface(VkInstance instance,
                                     VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
}

} // namespace vlkn
