// header
#include "keyboard_movement_controller.hpp"

// local
#include "mouse_movement_controller.hpp"
#include "vlkn_utils.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
// imgui
#include "imgui/imgui_impl_glfw.h"

// std
#include <limits>

namespace vlkn {
void KeyboardMovementController::keyboardCallback(GLFWwindow *const window,
                                                  const int key,
                                                  const int scancode,
                                                  const int action,
                                                  const int mods) {

  if (action == GLFW_PRESS && key == GLFW_KEY_C) [[unlikely]] {
    const bool imguiMode =
        glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;

    glfwSetInputMode(window, GLFW_CURSOR,
                     imguiMode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    imguiMode ? MouseMovementController::setMouseSensitivity(0.01f)
              : MouseMovementController::setMouseSensitivity(0.0f);
  }

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

// lock camera on game objects
// see docs/look_at_rotation_vector
void KeyboardMovementController::lookAt(
    const VlknGameObject::Map &gameObjects) {
  glm::vec3 direction{};
  for (auto i = GLFW_KEY_1; i < GLFW_KEY_9; i++) {
    const VlknGameObject::id_t id = i - GLFW_KEY_1;
    if (keys[i] && gameObjects.find(id) != gameObjects.end()) {
      direction = glm::normalize(gameObjects.at(id).transform.translation -
                                 viewerObject.transform.translation);
    }
  }

  if (nonZeroVector(direction)) {
    float yaw = std::atan2(direction.x, direction.z);
    float pitch = -std::asin(direction.y);

    pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));
    yaw = glm::mod(yaw, glm::two_pi<decltype(yaw)>());

    viewerObject.transform.rotation = glm::vec3(pitch, yaw, 0.0f);
  }
}

std::unordered_map<uint32_t, bool> KeyboardMovementController::keys{};

} // namespace vlkn