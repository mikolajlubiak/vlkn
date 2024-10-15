#pragma once

// local
#include "callbacks.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_window.hpp"

namespace vlkn {

class MouseMovementController {
public:
  void lookAround(VlknGameObject &gameObject);
  void zoom();

  float getFov() const { return fov; }

private:
  const float mouseSensitivity{0.01f};
  const float scrollSensitivity{0.1f};

  float mouseLastX{callbacks::mousePosX};
  float mouseLastY{callbacks::mousePosY};
  float mouseOffsetX{};
  float mouseOffsetY{};

  float fov{glm::radians(50.0f)};
};

} // namespace vlkn