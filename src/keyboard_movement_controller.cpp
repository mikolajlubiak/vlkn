// header
#include "keyboard_movement_controller.hpp"

// local
#include "vlkn_utils.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <limits>

namespace vlkn {
void KeyboardMovementController::keyboardCallback(GLFWwindow *const window,
                                                  const int key,
                                                  const int scancode,
                                                  const int action,
                                                  const int mods) {
  switch (action) {
  case GLFW_PRESS:
    keys[key] = true;
    break;
  case GLFW_RELEASE:
    keys[key] = false;
    break;
  default:
    break;
  }
}

void KeyboardMovementController::move(const float step) {
  const float yaw = viewerObject.transform.rotation.y;
  const float pitch = viewerObject.transform.rotation.x;

  const glm::vec3 forwardDir = glm::normalize(
      glm::vec3(std::sin(yaw) * std::cos(pitch), -std::sin(pitch),
                std::cos(yaw) * std::cos(pitch)));

  const glm::vec3 rightDir =
      glm::normalize(glm::cross(glm::vec3{0.0f, 1.0f, 0.0f}, forwardDir));

  const glm::vec3 upDir = glm::normalize(glm::cross(rightDir, forwardDir));

  glm::vec3 moveDir{0.0f};
  if (keys[moveForward]) {
    moveDir += forwardDir;
  }
  if (keys[moveBackward]) {
    moveDir -= forwardDir;
  }
  if (keys[moveRight]) {
    moveDir += rightDir;
  }
  if (keys[moveLeft]) {
    moveDir -= rightDir;
  }
  if (keys[moveUp]) {
    moveDir += upDir;
  }
  if (keys[moveDown]) {
    moveDir -= upDir;
  }

  if (nonZeroVector(moveDir)) {
    viewerObject.transform.translation +=
        speed * step * glm::normalize(moveDir);
  }
}

std::unordered_map<uint32_t, bool> KeyboardMovementController::keys{};

} // namespace vlkn