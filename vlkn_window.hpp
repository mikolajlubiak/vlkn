#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace vlkn {

class VlknWindow {
public:
  VlknWindow(const uint32_t w, const uint32_t h, const std::string name);
  ~VlknWindow();

  VlknWindow(const VlknWindow &) = delete;
  VlknWindow &operator=(const VlknWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }

  VkExtent2D getExtent() {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
  static void framebufferResizedCallback(GLFWwindow *window, int width,
                                         int height);
  void initWindow();

  uint32_t width, height;
  bool framebufferResized = false;
  const std::string windowName;
  GLFWwindow *window;
};

} // namespace vlkn
