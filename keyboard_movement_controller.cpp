// header
#include "keyboard_movement_controller.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>

// std
#include <limits>

namespace vlkn {
void KeyboardMovementController::moveInPlaneXZ(GLFWwindow *window,
                                               float deltaTime,
                                               VlknGameObject &gameObject) {
  glm::vec3 rotate{0.0f};

  if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) {
    rotate.y += 1.f;
  }
  if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) {
    rotate.y -= 1.f;
  }
  if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) {
    rotate.x += 1.f;
  }
  if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) {
    rotate.x -= 1.f;
  }

  if (glm::length2(rotate) > std::numeric_limits<float>::epsilon() *
                                 std::numeric_limits<float>::epsilon()) {
    gameObject.transform.rotation +=
        turnSpeed * deltaTime * glm::normalize(rotate);
  }

  gameObject.transform.rotation.x =
      glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
  gameObject.transform.rotation.y =
      glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

  float yaw = gameObject.transform.rotation.y;
  const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.0f, -forwardDir.x};
  const glm::vec3 upDir{0.0f, -1.0f, 0.0f};
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

  if (glm::length2(moveDir) > std::numeric_limits<float>::epsilon() *
                                  std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation +=
        movementSpeed * deltaTime * glm::normalize(moveDir);
  }
}
} // namespace vlkn