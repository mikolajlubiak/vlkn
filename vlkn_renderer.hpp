#pragma once

#include "vlkn_device.hpp"
#include "vlkn_swap_chain.hpp"
#include "vlkn_window.hpp"

#include <cassert>
#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

class VlknRenderer {
public:
  VlknRenderer(VlknWindow &window, VlknDevice &device);
  ~VlknRenderer();

  VlknRenderer(const VlknRenderer &) = delete;
  VlknRenderer &operator=(const VlknRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const {
    return vlknSwapChain->getRenderPass();
  }

  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted &&
           "Cannot get command buffer when frame is not in progress");
    return commandBuffers[currentFrameIndex];
  }

  uint32_t getFrameIndex() const {
    assert(isFrameStarted &&
           "Cannot get frame index when frame is not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  VlknWindow &vlknWindow;
  VlknDevice &vlknDevice;
  std::unique_ptr<VlknSwapChain> vlknSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  uint32_t currentFrameIndex{0};
  bool isFrameStarted{false};
};

} // namespace vlkn
