#include "app.hpp"
#include "vlkn_device.hpp"
#include "vlkn_pipeline.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <bits/fs_fwd.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace vlkn {

App::App() {
  createPipelineLayout();
  createPipeline();
  createCommandBuffers();
}

App::~App() {
  vkDestroyPipelineLayout(vlknDevice.device(), pipelineLayout, nullptr);
}

void App::run() {
  while (!vlknWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(vlknDevice.device());
}

void App::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  if (vkCreatePipelineLayout(vlknDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }
}

void App::createPipeline() {
  PipelineConfigInfo pipelineConfig = VlknPipeline::defaultPipelineConfigInfo(
      vlknSwapChain.width(), vlknSwapChain.height());
  pipelineConfig.renderPass = vlknSwapChain.getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  vlknPipeline = std::make_unique<VlknPipeline>(
      vlknDevice, "shaders/vert.spv", "shaders/frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {
  commandBuffers.resize(vlknSwapChain.imageCount());
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = vlknDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(vlknDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command buffers");
  }

  for (size_t i = 0; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vlknSwapChain.getRenderPass();
    renderPassInfo.framebuffer = vlknSwapChain.getFrameBuffer(i);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vlknSwapChain.getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vlknPipeline->bind(commandBuffers[i]);
    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[i]);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer");
    }
  }
}
void App::drawFrame() {
  uint32_t imageIndex;
  VkResult result = vlknSwapChain.acquireNextImage(&imageIndex);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire next image");
  }

  result = vlknSwapChain.submitCommandBuffers(&commandBuffers[imageIndex],
                                              &imageIndex);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to submit command buffer");
  }
}

} // namespace vlkn
