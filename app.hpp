#pragma once

#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_renderer.hpp"
#include "vlkn_window.hpp"

#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

class App {
public:
  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGH = 800;

  App();
  ~App();

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  void run();

private:
  void loadGameObjects();

  VlknWindow vlknWindow{WIDTH, HEIGH, "vlkn Demo"};
  VlknDevice vlknDevice{vlknWindow};
  VlknRenderer vlknRenderer{vlknWindow, vlknDevice};

  std::vector<VlknGameObject> gameObjects;
};

} // namespace vlkn
