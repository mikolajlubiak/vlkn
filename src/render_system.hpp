#pragma once

// local
#include "vlkn_camera.hpp"
#include "vlkn_device.hpp"
#include "vlkn_frame_info.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_pipeline.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <vector>

namespace vlkn {

class RenderSystem {
public:
  RenderSystem(VlknDevice &device, VkRenderPass renderPass);
  ~RenderSystem();

  RenderSystem(const RenderSystem &) = delete;
  RenderSystem &operator=(const RenderSystem &) = delete;

  void renderGameObjects(FrameInfo &frameInfo,
                         std::vector<VlknGameObject> &gameObjects);

private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  VlknDevice &vlknDevice;
  std::unique_ptr<VlknPipeline> vlknPipeline;
  VkPipelineLayout pipelineLayout;
};

} // namespace vlkn
