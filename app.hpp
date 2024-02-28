#pragma once

#include "vlkn_pipeline.hpp"
#include "vlkn_window.hpp"

#include <cstdint>

namespace vlkn {

class App {
public:
  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGH = 600;

  void run();

private:
  VlknWindow vlknWindow{WIDTH, HEIGH, "Hello Vulkan!"};
  VlknPipeline vlknPipeline{"shaders/vert.spv", "shaders/frag.spv"};
};

} // namespace vlkn
