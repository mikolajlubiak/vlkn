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
    // Get the forward vector based on the current rotation
    const glm::vec3 forwardDir = glm::rotate(viewerObject.transform.rotation,
                                             glm::vec3(0.0f, 0.0f, -1.0f));

    // Up vector always faces up (world up)
    constexpr glm::vec3 worldUp = glm::vec3(0.0f, -1.0f, 0.0f);

    // Calculate the right vector based on the forward direction
    const glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, worldUp));

    // Create quaternions for the rotations based on mouse movement
    const glm::quat pitchRotation =
        glm::angleAxis(mouseSensitivity * mouseOffsetY,
                       rightDir); // Rotate around local X-axis

    const glm::quat yawRotation = glm::angleAxis(
        mouseSensitivity * mouseOffsetX, worldUp); // Rotate around Y-axis

    // Combine the rotations (yaw first, then pitch)
    viewerObject.transform.rotation =
        yawRotation * pitchRotation * viewerObject.transform.rotation;

    // Normalize the quaternion to avoid drift
    viewerObject.transform.rotation =
        glm::normalize(viewerObject.transform.rotation);

    // Reset mouse offsets
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
float MouseMovementController::mouseSensitivity{0.01f};
float MouseMovementController::scrollSensitivity{0.1f};

} // namespace vlkn