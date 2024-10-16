#pragma once

// local
#include "vlkn_game_object.hpp"
#include "vlkn_window.hpp"

// std
#include <unordered_map>

namespace vlkn {

class KeyboardMovementController {
public:
  enum KeyMappings : uint32_t {
    moveLeft = GLFW_KEY_A,
    moveRight = GLFW_KEY_D,
    moveForward = GLFW_KEY_W,
    moveBackward = GLFW_KEY_S,
    moveUp = GLFW_KEY_E,
    moveDown = GLFW_KEY_Q,
    lookLeft = GLFW_KEY_LEFT,
    lookRight = GLFW_KEY_RIGHT,
    lookUp = GLFW_KEY_UP,
    lookDown = GLFW_KEY_DOWN,
    closeApp = GLFW_KEY_ESCAPE,
    lookAtCenter = GLFW_KEY_1,
  };

  KeyboardMovementController(VlknGameObject &gameObject)
      : gameObject(gameObject) {}

  bool shouldClose() { return keys[closeApp]; }

  void move(const float step);

  static void keyboardCallback(GLFWwindow *const window, const int key,
                               const int scancode, const int action,
                               const int mods);

private:
  VlknGameObject &gameObject;
  static std::unordered_map<uint32_t, bool> keys;
  static constexpr float speed{3.0f};
};

} // namespace vlkn