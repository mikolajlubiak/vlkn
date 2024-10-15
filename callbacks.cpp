// header
#include "callbacks.hpp"

namespace vlkn {

void callbacks::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  mousePosX = static_cast<float>(xpos);
  mousePosY = static_cast<float>(ypos);
}

void callbacks::scrollCallback(GLFWwindow *window, double xoffset,
                               double yoffset) {
  scrollOffsetY = static_cast<float>(yoffset);
}

float callbacks::mousePosX{};
float callbacks::mousePosY{};
float callbacks::scrollOffsetY{};

} // namespace vlkn