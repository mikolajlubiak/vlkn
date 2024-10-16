#pragma once

// local
#include "vlkn_game_object.hpp"
#include "vlkn_window.hpp"

namespace vlkn {

class MouseMovementController {
public:
  MouseMovementController(VlknGameObject &gameObject)
      : gameObject(gameObject) {}

  void lookAround();

  static float getFov() { return fov; }

  static void mouseCallback(GLFWwindow *const window, const double xpos,
                            const double ypos);

  static void scrollCallback(GLFWwindow *const window, const double xoffset,
                             const double yoffset);

private:
  VlknGameObject &gameObject;

  static constexpr float mouseSensitivity{0.01f};
  static constexpr float scrollSensitivity{0.1f};

  static float mouseLastX;
  static float mouseLastY;
  static float mouseOffsetX;
  static float mouseOffsetY;

  static float fov;
};

} // namespace vlkn