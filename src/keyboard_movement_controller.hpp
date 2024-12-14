#pragma once

// local
#include "vlkn_camera.hpp"
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
  };

  KeyboardMovementController(VlknCamera &camera, VlknGameObject &viewerObject)
      : camera(camera), viewerObject(viewerObject) {}

  bool shouldClose() { return keys[closeApp]; }

  void move(const float step);

  // lock camera on game objects
  // see docs/look_at_rotation_vector
  void lookAt(const VlknGameObject::Map &gameObjects);

  static void keyboardCallback(GLFWwindow *const window, const int key,
                               const int scancode, const int action,
                               const int mods);

  bool isKeyPressed(uint32_t key) { return keys[key]; }

private:
  VlknCamera &camera;
  VlknGameObject &viewerObject;
  static std::unordered_map<uint32_t, bool> keys;
  static constexpr float speed{3.0f};
};

} // namespace vlkn