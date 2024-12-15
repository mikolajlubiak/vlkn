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
  // Get the forward vector based on the current rotation
  const glm::vec3 forwardDir = glm::rotate(viewerObject.transform.rotation,
                                           glm::vec3(0.0f, 0.0f, -1.0f));

  // Get the up vector based on the current rotation
  const glm::vec3 upDir = glm::rotate(viewerObject.transform.rotation,
                                      glm::vec3(0.0f, -1.0f, 0.0f));

  // Right vector is the cross product of up and forward
  const glm::vec3 rightDir = glm::normalize(glm::cross(upDir, forwardDir));

  // Construct the move vector based on keyboard input
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
        speed * step *
        glm::normalize(moveDir); // Normalize the move vector to avoid sqrt(2)
                                 // times faster movement when going diagonally
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
    constexpr glm::vec3 upDir = glm::vec3(0.0f, 1.0f, 0.0f);

    // Create a quaternion that represents the rotation from the viewer's
    // forward direction to the target direction
    glm::quat rotation = glm::quatLookAtRH(direction, upDir);

    // Set the viewer's rotation to the calculated quaternion
    viewerObject.transform.rotation = glm::normalize(rotation);
  }
}

std::unordered_map<uint32_t, bool> KeyboardMovementController::keys{};

} // namespace vlkn