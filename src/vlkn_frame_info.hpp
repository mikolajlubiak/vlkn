#pragma once

// local
#include "vlkn_camera.hpp"
#include "vlkn_game_object.hpp"

// libs
#include <vulkan/vulkan.h>

// std
#include <array>

namespace vlkn {

constexpr std::size_t MAX_LIGHTS = 16;

struct PointLight {
  glm::vec4 position{};
  glm::vec4 color{};
};

struct GlobalUbo {
  glm::mat4 projection{1.0f};
  glm::mat4 view{1.0f};
  glm::mat4 inverseView{1.0f};
  glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
  std::array<PointLight, MAX_LIGHTS> pointLights{};
  std::size_t lightsNum = 0;
};

struct FrameInfo {
  std::uint32_t frameIndex;
  float frameDelta;
  float frameTime;
  VkCommandBuffer commandBuffer;
  VlknCamera &camera;
  VkDescriptorSet globalDescriptorSet;
  VlknGameObject::Map &gameObjects;
};

} // namespace vlkn