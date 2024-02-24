#pragma once

#include "vlkn_window.hpp"

#include <cstdint>

namespace vlkn {

class FirstApp {
public:
  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGH = 600;

  void run();

private:
  VlknWindow vlknWindow{WIDTH, HEIGH, "Hello Vulkan!"};
};

} // namespace vlkn
