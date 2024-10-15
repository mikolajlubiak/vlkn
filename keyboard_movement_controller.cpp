// header
#include "keyboard_movement_controller.hpp"

// local
#include "common.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <limits>

namespace vlkn {
void KeyboardMovementController::moveInPlaneXYZ(GLFWwindow *window, float step,
                                                VlknGameObject &gameObject) {
  float yaw = gameObject.transform.rotation.y;
  float pitch = gameObject.transform.rotation.x;

  const glm::vec3 forwardDir{std::sin(yaw) * std::cos(pitch), -std::sin(pitch),
                             std::cos(yaw) * std::cos(pitch)};
  const glm::vec3 rightDir =
      glm::normalize(glm::cross(glm::vec3{0.0f, 1.0f, 0.0f}, forwardDir));
  const glm::vec3 upDir = glm::normalize(glm::cross(rightDir, forwardDir));
  glm::vec3 moveDir{0.0f};

  if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) {
    moveDir += forwardDir;
  }
  if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
    moveDir -= forwardDir;
  }
  if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {
    moveDir += rightDir;
  }
  if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {
    moveDir -= rightDir;
  }
  if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
    moveDir += upDir;
  }
  if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
    moveDir -= upDir;
  }

  if (nonZeroVector(moveDir)) {
    gameObject.transform.translation += speed * step * glm::normalize(moveDir);
  }
}
} // namespace vlkn