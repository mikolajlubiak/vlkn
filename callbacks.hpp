#pragma once

// libs
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vlkn {

namespace callbacks {
void mouseCallback(GLFWwindow *window, double xpos, double ypos);

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

extern float mousePosX;
extern float mousePosY;
extern float scrollOffsetY;
}; // namespace callbacks

} // namespace vlkn