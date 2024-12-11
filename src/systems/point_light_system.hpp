#pragma once

// local
#include "vlkn_camera.hpp"
#include "vlkn_device.hpp"
#include "vlkn_frame_info.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_pipeline.hpp"

// libs
// GLM
#include <glm/glm.hpp>
// Vulkan
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <vector>

namespace vlkn {

class PointLightSystem {
public:
  PointLightSystem(VlknDevice &device, VkRenderPass renderPass,
                   VkDescriptorSetLayout globalSetLayout);
  ~PointLightSystem();

  PointLightSystem(const PointLightSystem &) = delete;
  PointLightSystem &operator=(const PointLightSystem &) = delete;

  void update(const FrameInfo &frameInfo, const glm::vec4 pointLightColor,
              GlobalUbo &ubo);
  void render(const FrameInfo &frameInfo, const glm::vec4 pointLightColor);

private:
  void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
  void createPipeline(VkRenderPass renderPass);

  VlknDevice &vlknDevice;
  std::unique_ptr<VlknPipeline> vlknPipeline;
  VkPipelineLayout pipelineLayout;
};

} // namespace vlkn
