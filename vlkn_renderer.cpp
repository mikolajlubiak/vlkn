#include "vlkn_renderer.hpp"
#include "vlkn_device.hpp"
#include "vlkn_swap_chain.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <bits/fs_fwd.h>
#include <cassert>
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

VlknRenderer::VlknRenderer(VlknWindow &window, VlknDevice &device)
    : vlknWindow(window), vlknDevice(device) {
  recreateSwapChain();
  createCommandBuffers();
}

VlknRenderer::~VlknRenderer() { freeCommandBuffers(); }

void VlknRenderer::recreateSwapChain() {
  VkExtent2D extent = vlknWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = vlknWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(vlknDevice.device());

  if (vlknSwapChain == nullptr) {
    vlknSwapChain = std::make_unique<VlknSwapChain>(vlknDevice, extent);
  } else {
    std::shared_ptr<VlknSwapChain> oldSwapChain = std::move(vlknSwapChain);

    vlknSwapChain =
        std::make_unique<VlknSwapChain>(vlknDevice, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*vlknSwapChain.get())) {
      throw std::runtime_error("Swap chain image or depth format have changed");
    }
  }
}

void VlknRenderer::createCommandBuffers() {
  commandBuffers.resize(VlknSwapChain::MAX_FRAMES_IN_FLIGHT);
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = vlknDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(vlknDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command buffers");
  }
}

void VlknRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(vlknDevice.device(), vlknDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

VkCommandBuffer VlknRenderer::beginFrame() {
  assert(!isFrameStarted &&
         "Cant call beginFrame while frame is already in progress");

  VkResult result = vlknSwapChain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire next image");
  }

  isFrameStarted = true;

  VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin buffer");
  }

  return commandBuffer;
}

void VlknRenderer::endFrame() {
  assert(isFrameStarted && "Cant call endFrame while frame is not in progress");

  VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer");
  }

  VkResult result =
      vlknSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      vlknWindow.wasWindowResized()) {
    vlknWindow.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image");
  }

  isFrameStarted = false;
  currentFrameIndex =
      (currentFrameIndex + 1) % VlknSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void VlknRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Cant call beginSwapChainRenderPass while frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Cant begin render pass on command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = vlknSwapChain->getRenderPass();
  renderPassInfo.framebuffer = vlknSwapChain->getFrameBuffer(currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = vlknSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width =
      static_cast<float>(vlknSwapChain->getSwapChainExtent().width);
  viewport.height =
      static_cast<float>(vlknSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, vlknSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VlknRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Cant call endSwapChainRenderPass while frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Cant end render pass on command buffer from a different frame");

  vkCmdEndRenderPass(commandBuffer);
}

} // namespace vlkn
