#pragma once

// local
#include "vlkn_game_object.hpp"
#include "vlkn_window.hpp"

namespace vlkn {

class MouseMovementController {
public:
  MouseMovementController(VlknGameObject &viewerObject)
      : viewerObject(viewerObject) {}

  void lookAround();

  static float getFov() { return fov; }

  static void mouseCallback(GLFWwindow *const window, const double xpos,
                            const double ypos);

  static void scrollCallback(GLFWwindow *const window, const double xoffset,
                             const double yoffset);

  static void setMouseSensitivity(const float sensitivity) {
    mouseSensitivity = sensitivity;
  }

private:
  VlknGameObject &viewerObject;

  static float mouseSensitivity;
  static float scrollSensitivity;

  static float mouseLastX;
  static float mouseLastY;
  static float mouseOffsetX;
  static float mouseOffsetY;

  static float fov;
};

} // namespace vlkn