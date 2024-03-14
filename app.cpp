#include "app.hpp"
#include "vlkn_device.hpp"
#include "vlkn_model.hpp"
#include "vlkn_pipeline.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <bits/fs_fwd.h>
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

App::App() {
  loadModels();
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
    vlknModel->bind(commandBuffers[i]);
    vlknModel->draw(commandBuffers[i]);

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

void App::sierpinski(std::vector<VlknModel::Vertex> &vertices, int depth,
                     glm::vec2 left, glm::vec2 right, glm::vec2 top,
                     glm::vec3 leftColor, glm::vec3 rightColor,
                     glm::vec3 topColor) {
  if (depth <= 0) {
    vertices.emplace_back(top, topColor);
    vertices.emplace_back(right, rightColor);
    vertices.emplace_back(left, leftColor);
  } else {
    glm::vec2 leftTop = 0.5f * (left + top);
    glm::vec2 rightTop = 0.5f * (right + top);
    glm::vec2 leftRight = 0.5f * (left + right);
    sierpinski(vertices, depth - 1, left, leftRight, leftTop, leftColor,
               rightColor, topColor);
    sierpinski(vertices, depth - 1, leftRight, right, rightTop, leftColor,
               rightColor, topColor);
    sierpinski(vertices, depth - 1, leftTop, rightTop, top, leftColor,
               rightColor, topColor);
  }
}

void App::loadModels() {
  const uint8_t VERT_COUNT = 1;
  std::vector<VlknModel::Vertex> vertices;
  vertices.reserve(VERT_COUNT);
  sierpinski(vertices, VERT_COUNT, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f},
             {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
  vlknModel = std::make_unique<VlknModel>(vlknDevice, vertices);
}

} // namespace vlkn
