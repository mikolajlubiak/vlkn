#pragma once

// local
#include "vlkn_descriptors.hpp"
#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_renderer.hpp"
#include "vlkn_window.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <cstdint>
#include <vector>

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

  VlknWindow vlknWindow{WIDTH, HEIGH, "vlkn"};
  VlknDevice vlknDevice{vlknWindow};
  VlknRenderer vlknRenderer{vlknWindow, vlknDevice};

  std::unique_ptr<VlknDescriptorPool> globalPool{};
  std::unique_ptr<VlknDescriptorPool> imguiPool{};

  VlknGameObject::Map gameObjects;
};

} // namespace vlkn
