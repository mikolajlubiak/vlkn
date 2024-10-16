// header
#include "vlkn_window.hpp"

// local
#include "keyboard_movement_controller.hpp"
#include "mouse_movement_controller.hpp"

// libs
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

// std
#include <cstdint>
#include <stdexcept>
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
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);

  glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);

  glfwSetCursorPosCallback(window, MouseMovementController::mouseCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  glfwSetScrollCallback(window, MouseMovementController::scrollCallback);

  glfwSetKeyCallback(window, KeyboardMovementController::keyboardCallback);
}

void VlknWindow::createWindowSurface(VkInstance instance,
                                     VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
}

void VlknWindow::framebufferResizedCallback(GLFWwindow *window, int width,
                                            int height) {
  VlknWindow *vlknWindow =
      reinterpret_cast<VlknWindow *>(glfwGetWindowUserPointer(window));
  vlknWindow->framebufferResized = true;
  vlknWindow->width = static_cast<uint32_t>(width);
  vlknWindow->height = static_cast<uint32_t>(height);
}
} // namespace vlkn
