// header
#include "mouse_movement_controller.hpp"

// local
#include "vlkn_utils.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <limits>

namespace vlkn {

void MouseMovementController::lookAround() {
  if (mouseOffsetX != 0.0f || mouseOffsetY != 0.0f) {
    viewerObject.transform.rotation.x += mouseSensitivity * mouseOffsetY;
    viewerObject.transform.rotation.y += mouseSensitivity * mouseOffsetX;

    viewerObject.transform.rotation.x =
        glm::clamp(viewerObject.transform.rotation.x, glm::radians(-89.0f),
                   glm::radians(89.0f));

    viewerObject.transform.rotation.y =
        glm::mod(viewerObject.transform.rotation.y,
                 glm::two_pi<decltype(viewerObject.transform.rotation.y)>());

    mouseOffsetX = 0.0f;
    mouseOffsetY = 0.0f;
  }
}

void MouseMovementController::mouseCallback(GLFWwindow *const window,
                                            const double xpos,
                                            const double ypos) {
  mouseOffsetX = static_cast<float>(xpos) - mouseLastX;
  mouseOffsetY = mouseLastY - static_cast<float>(ypos);
  mouseLastX = static_cast<float>(xpos);
  mouseLastY = static_cast<float>(ypos);
}

void MouseMovementController::scrollCallback(GLFWwindow *const window,
                                             const double xoffset,
                                             const double yoffset) {
  fov -= scrollSensitivity * static_cast<float>(yoffset);
  fov = glm::clamp(fov, glm::radians(1.0f), glm::radians(89.0f));
}

float MouseMovementController::fov{glm::radians(50.0f)};
float MouseMovementController::mouseOffsetX{};
float MouseMovementController::mouseOffsetY{};
float MouseMovementController::mouseLastX{};
float MouseMovementController::mouseLastY{};

} // namespace vlkn