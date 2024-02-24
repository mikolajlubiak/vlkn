#pragma once

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

private:
  void initWindow();

  const uint32_t width, height;
  const std::string windowName;
  GLFWwindow *window;
};

} // namespace vlkn
