#pragma once

// local
#include "vlkn_game_object.hpp"
#include "vlkn_window.hpp"

namespace vlkn {

class KeyboardMovementController {
public:
  struct KeyMappings {
    uint32_t moveLeft = GLFW_KEY_A;
    uint32_t moveRight = GLFW_KEY_D;
    uint32_t moveForward = GLFW_KEY_W;
    uint32_t moveBackward = GLFW_KEY_S;
    uint32_t moveUp = GLFW_KEY_E;
    uint32_t moveDown = GLFW_KEY_Q;
    uint32_t lookLeft = GLFW_KEY_LEFT;
    uint32_t lookRight = GLFW_KEY_RIGHT;
    uint32_t lookUp = GLFW_KEY_UP;
    uint32_t lookDown = GLFW_KEY_DOWN;
  };

  void moveInPlaneXZ(GLFWwindow *window, float deltaTime,
                     VlknGameObject &gameObject);

  KeyMappings keys{};
  float movementSpeed{3.0f};
  float turnSpeed{3.0f};
};

} // namespace vlkn