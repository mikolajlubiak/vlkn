// header
#include "mouse_movement_controller.hpp"

// local
#include "callbacks.hpp"
#include "common.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <limits>

namespace vlkn {

void MouseMovementController::lookAround(VlknGameObject &gameObject) {
  glm::vec3 rotate{0.0f};

  mouseOffsetX = callbacks::mousePosX - mouseLastX;
  mouseOffsetY = mouseLastY - callbacks::mousePosY;
  mouseLastX = callbacks::mousePosX;
  mouseLastY = callbacks::mousePosY;

  rotate.x += mouseOffsetY;
  rotate.y += mouseOffsetX;

  if (nonZeroVector(rotate)) {
    gameObject.transform.rotation += mouseSensitivity * glm::normalize(rotate);
  }

  gameObject.transform.rotation.x =
      glm::clamp(gameObject.transform.rotation.x, glm::radians(-89.0f),
                 glm::radians(89.0f));
  gameObject.transform.rotation.y =
      glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
}

void MouseMovementController::zoom() {
  fov -= callbacks::scrollOffsetY * scrollSensitivity;
  fov = glm::clamp(fov, glm::radians(1.0f), glm::radians(89.0f));
  callbacks::scrollOffsetY = 0.0f;
}

} // namespace vlkn