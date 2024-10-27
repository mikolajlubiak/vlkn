#pragma once

// local
#include "vlkn_camera.hpp"

// libs
#include <vulkan/vulkan.h>

namespace vlkn {

struct FrameInfo {
  std::uint32_t frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  VlknCamera &camera;
};

} // namespace vlkn