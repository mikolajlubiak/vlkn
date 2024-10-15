// header
#include "mouse_movement_controller.hpp"

// local
#include "common.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <limits>

namespace vlkn {

void MouseMovementController::lookAround(VlknGameObject &gameObject) {
  if (mouseOffsetX != 0.0f || mouseOffsetY != 0.0f) {
    gameObject.transform.rotation.x += mouseSensitivity * mouseOffsetY;
    gameObject.transform.rotation.y += mouseSensitivity * mouseOffsetX;
    gameObject.transform.rotation.x =
        glm::clamp(gameObject.transform.rotation.x, glm::radians(-89.0f),
                   glm::radians(89.0f));
    gameObject.transform.rotation.y =
        glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
    mouseOffsetX = 0.0f;
    mouseOffsetY = 0.0f;
  }
}

void MouseMovementController::mouseCallback(GLFWwindow *window, double xpos,
                                            double ypos) {
  mouseOffsetX = static_cast<float>(xpos) - mouseLastX;
  mouseOffsetY = mouseLastY - static_cast<float>(ypos);
  mouseLastX = static_cast<float>(xpos);
  mouseLastY = static_cast<float>(ypos);
}

void MouseMovementController::scrollCallback(GLFWwindow *window, double xoffset,
                                             double yoffset) {
  fov -= scrollSensitivity * static_cast<float>(yoffset);
  fov = glm::clamp(fov, glm::radians(1.0f), glm::radians(89.0f));
}

float MouseMovementController::fov{glm::radians(50.0f)};
float MouseMovementController::mouseOffsetX{};
float MouseMovementController::mouseOffsetY{};
float MouseMovementController::mouseLastX{};
float MouseMovementController::mouseLastY{};

} // namespace vlkn