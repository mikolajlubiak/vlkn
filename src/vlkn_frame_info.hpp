#pragma once

// local
#include "vlkn_camera.hpp"
#include "vlkn_game_object.hpp"

// libs
#include <vulkan/vulkan.h>

namespace vlkn {

struct FrameInfo {
  std::uint32_t frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  VlknCamera &camera;
  VkDescriptorSet globalDescriptorSet;
  VlknGameObject::Map &gameObjects;
};

} // namespace vlkn