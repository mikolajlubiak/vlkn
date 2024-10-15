#pragma once

// local
#include "vlkn_game_object.hpp"
#include "vlkn_window.hpp"

namespace vlkn {

class MouseMovementController {
public:
  void lookAround(VlknGameObject &gameObject);

  static float getFov() { return fov; }

  static void mouseCallback(GLFWwindow *window, double xpos, double ypos);

  static void scrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);

private:
  static constexpr float mouseSensitivity{0.01f};
  static constexpr float scrollSensitivity{0.1f};

  static float mouseLastX;
  static float mouseLastY;
  static float mouseOffsetX;
  static float mouseOffsetY;

  static float fov;
};

} // namespace vlkn